/*
skiplist.h
Inline Skip list
- Splicing allows for moving elements between different data structures
  without copying or reallocation
- Both do operations in O(log n)
*/

#pragma once
#include <stdlib.h>
#include <assert.h>

#include <algorithm>
#include <atomic>
#include <type_traits>

#include "../../utils/comparator.h"
#include "../memory/allocator.h"

template <typename Key, class Comparator>
class SkipList {
private:
  struct Node;

 public:
  // Create a new SkipList object that will use "cmp" for comparing keys,
  // and will allocate memory using "*allocator".  Objects allocated in the
  // allocator must remain allocated for the lifetime of the skiplist object.
  explicit SkipList(Comparator cmp, Allocator* allocator,
                    int32_t max_height = 12, int32_t branching_factor = 4);
  // No copying allowed
  SkipList(const SkipList&) = delete;
  void operator=(const SkipList&) = delete;

  // Insert key into the list.
  // REQUIRES: nothing that compares equal to key is currently in the list.
  void Insert(const Key& key);

  // Returns true iff an entry that compares equal to key is in the list.
  bool Contains(const Key& key) const;

  // Return estimated number of entries smaller than `key`.
  uint64_t EstimateCount(const Key& key) const;

  // Iteration over the contents of a skip list
  class Iterator {
   public:
    // Initialize an iterator over the specified list.
    // The returned iterator is not valid.
    explicit Iterator(const SkipList* list);

    // Change the underlying skiplist used for this iterator
    // This enables us not changing the iterator without deallocating
    // an old one and then allocating a new one
    void SetList(const SkipList* list);

    // Returns true iff the iterator is positioned at a valid node.
    bool Valid() const;

    // Returns the key at the current position.
    // REQUIRES: Valid()
    const Key& key() const;

    // Advances to the next position.
    // REQUIRES: Valid()
    void Next();

    // Advances to the previous position.
    // REQUIRES: Valid()
    void Prev();

    // Advance to the first entry with a key >= target
    void Seek(const Key& target);

    // Retreat to the last entry with a key <= target
    void SeekForPrev(const Key& target);

    // Position at the first entry in list.
    // Final state of iterator is Valid() iff list is not empty.
    void SeekToFirst();

    // Position at the last entry in list.
    // Final state of iterator is Valid() iff list is not empty.
    void SeekToLast();

   private:
    const SkipList* list_;
    Node* node_;
    // Intentionally copyable
  };

 private:
  const uint16_t kMaxHeight_;
  const uint16_t kBranching_;
  const uint32_t kScaledInverseBranching_;

  // Immutable after construction
  Comparator const compare_;
  Allocator* const allocator_;  // Allocator used for allocations of nodes

  Node* const head_;

  // Modified only by Insert().  Read racily by readers, but stale
  // values are ok.
  std::atomic<int> max_height_;  // Height of the entire list

  // Used for optimizing sequential insert patterns.  Tricky.  prev_[i] for
  // i up to max_height_ is the predecessor of prev_[0] and prev_height_
  // is the height of prev_[0].  prev_[0] can only be equal to head before
  // insertion, in which case max_height_ and prev_height_ are 1.
  Node** prev_;
  int32_t prev_height_;

  inline int GetMaxHeight() const {
    return max_height_.load(std::memory_order_relaxed);
  }

  Node* NewNode(const Key& key, int height);
  int RandomHeight();
  bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }
  bool LessThan(const Key& a, const Key& b) const {
    return (compare_(a, b) < 0);
  }

  // Return true if key is greater than the data stored in "n"
  bool KeyIsAfterNode(const Key& key, Node* n) const;

  // Returns the earliest node with a key >= key.
  // Return nullptr if there is no such node.
  Node* FindGreaterOrEqual(const Key& key) const;

  // Return the latest node with a key < key.
  // Return head_ if there is no such node.
  // Fills prev[level] with pointer to previous node at "level" for every
  // level in [0..max_height_-1], if prev is non-null.
  Node* FindLessThan(const Key& key, Node** prev = nullptr) const;

  // Return the last node in the list.
  // Return head_ if list is empty.
  Node* FindLast() const;
};

// Implementation details below
template <typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
  explicit Node(const Key& k) : key(k) {}

  Key const key;

  Node* Next(int n) {
    assert(n >= 0);
    return (next_[n].load(std::memory_order_acquire));
  }

  Node* 

  private:
    // 
    std::atomic<Node*> next_[1];
}
