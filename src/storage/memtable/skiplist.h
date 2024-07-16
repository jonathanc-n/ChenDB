/*
skiplist.h
Inline Skip list
- Splicing allows for moving elements between different data structures
  without copying or reallocation
*/

#pragma once
#include <stdlib.h>
#include <assert.h>

#include <algorithm>
#include <atomic>
#include <type_traits>

#include "../../utils/comparator.h"
#include "../memory/allocator.h"

template <typename Key> 
class SkipList {
private:
  struct Node;
  struct Splice

public:
  

  static const uint16_t kMaxPossibleHeight = 32;
  // Intializes the skiplist with a max height and branching factor
  explicit SkipList(Allocator* cmp, int32_t max_height = 16, int32_t branching_factor = 4);
  SkipList(const SkipList&) = delete;

  // Removes copying on the skiplist object
  SkipList(const SkipList&) = delete;
  SkipList& operator=(const SkipList&) = delete;

  Splice* AllocateSplice();

  Splice* AllocateSpliceOnHeap();

  char* AllocateKey(size_t key_size);

  // Inserts key in list
  bool Insert(const char*key);

  // Returns true if key is found in list
  bool InsertWithHint(const char* key, void** hint);

  bool InsertWithHintConcurrently(const char* key, void** hint);
// Like Insert, but external synchronization is not required.
  bool InsertConcurrently(const char* key);

  // Inserts a node into the skip list.  key must have been allocated by
  // AllocateKey and then filled in by the caller.  If UseCAS is true,
  // then external synchronization is not required, otherwise this method
  // may not be called concurrently with any other insertions.
  //
  // Regardless of whether UseCAS is true, the splice must be owned
  // exclusively by the current thread.  If allow_partial_splice_fix is
  // true, then the cost of insertion is amortized O(log D), where D is
  // the distance from the splice to the inserted key (measured as the
  // number of intervening nodes).  Note that this bound is very good for
  // sequential insertions!  If allow_partial_splice_fix is false then
  // the existing splice will be ignored unless the current key is being
  // inserted immediately after the splice.  allow_partial_splice_fix ==
  // false has worse running time for the non-sequential case O(log N),
  // but a better constant factor.
  template <bool UseCAS>
  bool Insert(const char* key, Splice* splice, bool allow_partial_splice_fix);

  // Returns true iff an entry that compares equal to key is in the list.
  bool Contains(const char* key) const;

  // Return estimated number of entries smaller than `key`.
  uint64_t EstimateCount(const char* key) const;

  // Validate correctness of the skip-list.
  void TEST_Validate() const;

  // Iteration over the contents of a skip list
  class Iterator {
   public:
    // Initialize an iterator over the specified list.
    // The returned iterator is not valid.
    explicit Iterator(const InlineSkipList* list);

    // Change the underlying skiplist used for this iterator
    // This enables us not changing the iterator without deallocating
    // an old one and then allocating a new one
    void SetList(const InlineSkipList* list);

    // Returns true iff the iterator is positioned at a valid node.
    bool Valid() const;

    // Returns the key at the current position.
    // REQUIRES: Valid()
    const char* key() const;

    // Advances to the next position.
    // REQUIRES: Valid()
    void Next();

    // Advances to the previous position.
    // REQUIRES: Valid()
    void Prev();

    // Advance to the first entry with a key >= target
    void Seek(const char* target);

    // Retreat to the last entry with a key <= target
    void SeekForPrev(const char* target);

    // Advance to a random entry in the list.
    void RandomSeek();

    // Position at the first entry in list.
    // Final state of iterator is Valid() iff list is not empty.
    void SeekToFirst();

    // Position at the last entry in list.
    // Final state of iterator is Valid() iff list is not empty.
    void SeekToLast();

   private:
    const InlineSkipList* list_;
    Node* node_;
    // Intentionally copyable
  };

 private:
  const uint16_t kMaxHeight_;
  const uint16_t kBranching_;
  const uint32_t kScaledInverseBranching_;

  Allocator* const allocator_;  // Allocator used for allocations of nodes
  // Immutable after construction
  Comparator const compare_;
  Node* const head_;

  // Modified only by Insert().  Read racily by readers, but stale
  // values are ok.
  std::atomic<int> max_height_;  // Height of the entire list

  // seq_splice_ is a Splice used for insertions in the non-concurrent
  // case.  It caches the prev and next found during the most recent
  // non-concurrent insertion.
  Splice* seq_splice_;

  inline int GetMaxHeight() const {
    return max_height_.load(std::memory_order_relaxed);
  }

  int RandomHeight();

  Node* AllocateNode(size_t key_size, int height);

  bool Equal(const char* a, const char* b) const {
    return (compare_(a, b) == 0);
  }

  bool LessThan(const char* a, const char* b) const {
    return (compare_(a, b) < 0);
  }

  // Return true if key is greater than the data stored in "n".  Null n
  // is considered infinite.  n should not be head_.
  bool KeyIsAfterNode(const char* key, Node* n) const;
  bool KeyIsAfterNode(const DecodedKey& key, Node* n) const;

  // Returns the earliest node with a key >= key.
  // Return nullptr if there is no such node.
  Node* FindGreaterOrEqual(const char* key) const;

  // Return the latest node with a key < key.
  // Return head_ if there is no such node.
  // Fills prev[level] with pointer to previous node at "level" for every
  // level in [0..max_height_-1], if prev is non-null.
  Node* FindLessThan(const char* key, Node** prev = nullptr) const;

  // Return the latest node with a key < key on bottom_level. Start searching
  // from root node on the level below top_level.
  // Fills prev[level] with pointer to previous node at "level" for every
  // level in [bottom_level..top_level-1], if prev is non-null.
  Node* FindLessThan(const char* key, Node** prev, Node* root, int top_level,
                     int bottom_level) const;

  // Return the last node in the list.
  // Return head_ if list is empty.
  Node* FindLast() const;

  // Returns a random entry.
  Node* FindRandomEntry() const;

  // Traverses a single level of the list, setting *out_prev to the last
  // node before the key and *out_next to the first node after. Assumes
  // that the key is not present in the skip list. On entry, before should
  // point to a node that is before the key, and after should point to
  // a node that is after the key.  after should be nullptr if a good after
  // node isn't conveniently available.
  template <bool prefetch_before>
  void FindSpliceForLevel(const DecodedKey& key, Node* before, Node* after,
                          int level, Node** out_prev, Node** out_next);

  // Recomputes Splice levels from highest_level (inclusive) down to
  // lowest_level (inclusive).
  void RecomputeSpliceLevels(const DecodedKey& key, Splice* splice,
                             int recompute_level);
};

// Implementation 

template <class Comparator>
struct InlineSkipList<Comparator>::Splice {
  int height_ = 0;
  Node** prev_;
  Node** next_;
};
