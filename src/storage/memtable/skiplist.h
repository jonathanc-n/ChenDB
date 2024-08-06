#pragma once
#include <assert.h>
#include <stdlib.h>

#include <atomic>

template <typename Key, class Comparator>
class SkipList {
  public:
    explicit SkipList();
    SkipList(const SkipList&) = delete;
    void operator = (const SkipLiset&) = delete;

    void Insert(const Key& key);
    void Remove(const Key& key);
    void Contains(const Key& key);

    class Iterator {
      public:
        explicit Iterator(const SkipList* list);
        bool ValidNode();
        void SetList(const SkipList* list);
        const Key& Key() const;
        void Next();
        void Prev();
        void BackNext();
        void BackPrev();
        
        void Seek(const Key& target);

        // Backwards
        void SeekPrev(const Key& target);
        void SeekToFirst();
        void SeekToLast();
      private:
        const SkipList* list_;
        Node* node_;
    }
  
  private:
    struct Node;

    Comparator const compare_;

    Node* const head_;
    Node* const tail_;

    std::atomic<int> max_height_;

    inline int GetMaxHeight(){
      return max_height_.load(std::memory_order_relaxed);
    }

    Node* NewNode(const Key& key, int height);
    int RandomHeight();

    bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }

    bool LessThan(const Key& a, const Key& b) const {
      return (compare_(a, b) < 0);
    }

    bool KeyIsAfterNode(const Key& key, Node* n) const;

    // For backwards iteration
    bool KeyIsBeforeNode(const Key& key, Node* n)

    Node* FindGreaterOrEqual(const Key& key) const;
    Node* FindLessThan(const Key& key, Node** prev = nullptr) const;

    //
    Node* FindLessOrEqual(const Key& key) const;
    Node* FindGreaterThan(const Key& key, Node** prev = nullptr) const;

    Node* FindLast() const;
}

// Note that in the future, when a node is being placed, both the backward
// and forward iterations should be accounted for.
template <typename Key, class Comparator> 
struct SkipList<Key, Comparator>::Node {
  explicit Node(const Key& k) : key(k) {}

  Key const key;

  Node* Next(int n) {
    assert(n >= 0);
    return (next_[n].load(std::memory_order_acquire));
  }

  // For backward iterations
  Node* Prev(int n) {
    assert(n >= 0);
    return (next_[n].load(std::memory_order_acquire));
  }

  void SetNext(int n, Node* new_node) {
    assert(n >= 0);
    next_[n].store(new_node, std::memory_order_release);
  }

  Node* SetPrev(int n, Node* new_node) {
    assert(n >= 0);
    prev_[n].store(new_node, std::memory_order_release);
  }

  private:
    std::atomic<Node*> next_[1];
    std::atomic<Node*> prev_[1];
}

// TODO: allocator for instantiating a new node

template <typename Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
  SetList(list);
}

template <typename Key, class Comparator>
inline bool SkipList<Key, Comparator>::Iterator::ValidNode(const Node* n) const {
  return node_ != nullptr;
}

template <typename Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::SetList(const SkipList* list) {
  list_ = list;
  node_ = nullptr;
}

template <typename Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::Key() {
  assert(ValidNode());
  return node_->key;
}

// TODO: here
template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Iterator::Next() {
  assert(ValidNode());
  node_->Next(0);
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Iterator::Prev() {
  assert(ValidNode());
  node_->Prev(0);
}


