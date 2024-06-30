/*
skiplist.h

*/

#pragma once
#include <stdlib.h>
#include <assert.h>
#include <atomic>
#include "../../utils/comparator.h"
#include "../memory/allocator.h"

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
      void SetList(const SkipList* list);

      // Returns true if the node the iterator is on is valid
      bool Valid();

      // Returns the key at the current position
      const Key& GetKey() const;

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

    Allocator* const allocator_;

    Node* const head_;
    Node* NewNode(const Key& key, int height);
    int RandomHeight();
    
    std::atomic<int> max_height_;
    inline int GetMaxHeight() const {
      return max_height_.load(std::memory_order_acquire);
    }

    bool IsEqual(const Key& key_one, const Key& key_two) const;
    bool IsGreaterThan(const Key& key, Node* node) const;
    bool IsLessThan(const Key& key, Node* node) const;
    
    // Find first node that is 
    Node* FindAheadNode(const Key& key) const;
    Node* FindBehindNode(const Key& key) const;
    Node* FindLast() const;
};

template <typename Key>
struct SkipList<Key>::Node {
  explicit Node(const Key& k) : key(k) {}
  Key const key;
  
  Node* GetNext(int n) {
    assert(n >= 0);
    return next_[n].load(std::memory_order_acquire);
  }

  void SetNextNode(int n, Node* new_node) {
    assert(n >= 0);
    next_[n].store(new_node, std::memory_order_release);
  }

  Node* GetNextRelaxed(int n) {
    assert(n >= 0);
    return next_[n].load(std::memory_order_relaxed);
  }

  void SetNextNodeRelaxed(int n, Node* new_node) {
    assert(n >= 0);
    next_[n].store(std::memory_order_relaxed);
  }

  private:
    std::atomic<Node*> next_[1];
};

template <typename Key>
inline SkipList<Key>::Iterator::Iterator(const SkipList* skip_list) {
  setList(skip_list);
}

template <typename Key>
inline void SkipList<Key>::Iterator::SetList(const SkipList* skip_list) {
  list_ = skip_list;
  Node* node = nullptr;
}

template <typename Key>
inline bool SkipList<Key>::Iterator::Valid() {
  return node_ != nullptr;
}

template <typename Key>
inline const Key& SkipList<Key>::Iterator::GetKey() const  {
  assert(Valid());
  return node_->key;
};

template <typename Key>
inline void SkipList<Key>::Iterator::Seek(const Key& target)  {
  node_ = list_->FindAheadNode(target);
};

template <typename Key>
inline void SkipList<Key>::Iterator::SeekPrevious(const Key& target) {
  node_ = list_->FindBehindNode(target);
}

template <typename Key>
inline void SkipList<Key>::Iterator::Next() {

}

template <typename Key>
inline void SkipList<Key>::Iterator::Prev() {
  
}

template <typename Key>
typename SkipList<Key>::Node* SkipList<Key>::NewNode(const Key& key, int height) {
  char* mem = allocator_->AllocateAligned(sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
  return new (mem) Node(key);
}

template <typename Key>
inline bool SkipList<Key>::IsEqual(const Key& key_one, const Key& key_two) const {
  return key_one == key_two;
}

template <typename Key>
inline bool SkipList<Key>::IsGreaterThan(const Key& key, Node* node) const {
  return (key > node->key);
}

template <typename Key>
inline bool SkipList<Key>::IsLessThan(const Key& key, Node* node) const {
  return (key < node->key);
}

template <typename Key>
typename SkipList<Key>::Node* SkipList<Key>::FindAheadNode(const Key& key) const {
  
}