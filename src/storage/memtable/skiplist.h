
#pragma once
#include <stdlib.h>
#include <atomic>
#include "../../utils/comparator.h"

template <typename Key> 
class SkipList {
private:
  struct Node;
public:
  // Intializes the skiplist with a max height and branching factor
  explicit SkipList(int32_t max_height = 16, int32_t branching_factor = 4);
  SkipList(const SkipList&) = delete;

  // Removes copying on the skiplist object
  void operator=(const SkipList&) = delete;

  // Inserts key in list
  void Insert(Key& key);

  // Returns true if key is found in list
  bool Contains(Key& key);

  class Iterator {
    public:
      explicit Iterator(const SkipList* list);

      // Allows for changing the iterator to a new list
      void SetList();

      // Returns the key at the current position
      const Key& GetKey();

      // Returns true if the node the iterator is on is valid
      bool Valid();

      // Place iterator at the first entry where key >= target
      void Seek(const Key& target);

      // Place iterator with a key <= target
      void SeekPrevious(const Key& target);

      void Next();
      void Prev();
      void PositionAtStart();
      void PositionAtEnd();

    private:
      const SkipList* list_;
      Node* node_;
  };

  private:
    int32_t max_skiplist_height_;

    Node* const head_;
    Node* NewNode(Key& key, int height);
    int RandomHeight();
    
    
    bool IsEqual(const Key& key_one, const Key& key_two) const;
    bool IsGreaterThan(const Key& key, Node* node) const;
    bool IsLessThan(const Key& key, Node* node) const;

    std::atomic<int> max_height_;

    Node* FindAheadNode(const Key& key) const;
    Node* FindLessThan(const Key& key) const;
    Node* FindLast() const;
};
