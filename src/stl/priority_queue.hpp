#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>

#include "exceptions.hpp"

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for
 * certain data. In such cases, any ongoing operation should be terminated, and
 * the priority queue should be restored to its original state before the
 * operation began.
 */
template <typename T, class Compare = std::less<T>>
class priority_queue {
 private:
  struct Node {
    T data;
    Node *left, *right, *parent;
    int npl;
    Node() : left(nullptr), right(nullptr), parent(nullptr), npl(-1) {}
    Node(const T& data_, Node* left_ = nullptr, Node* right_ = nullptr,
         Node* parent_ = nullptr)
        : data(data_), left(left_), right(right_), parent(parent_), npl(-1) {}
  };
  Node* copytree(Node* src, Node* parent) {
    if (src == nullptr) {
      return nullptr;
    }
    Node* newNode = new Node(src->data);
    newNode->left = copytree(src->left, newNode);
    newNode->right = copytree(src->right, newNode);
    newNode->parent = parent;
    newNode->npl = src->npl;
    return newNode;
  };
  void clear(Node*& root) {
    if (root == nullptr) {
      return;
    }
    clear(root->left);
    clear(root->right);
    delete root;
    root = nullptr;
  };
  Node* merge_heap(Node* x, Node* y) {
    if (x == nullptr) {
      return y;
    }
    if (y == nullptr) {
      return x;
    }
    Compare cmp;
    bool swap_flag;
    try {
      swap_flag = cmp(x->data, y->data);
    } catch (...) {
      throw;
    }
    if (swap_flag) {
      std::swap(x, y);
    }
    try {
      x->right = merge_heap(x->right, y);
    } catch (...) {
      if (swap_flag) {
        std::swap(y, x);
      }
      throw;
    }

    if (x->right) {
      x->right->parent = x;
    }
    if (!x->left) {
      std::swap(x->left, x->right);
    } else {
      int left_npl = x->left->npl;
      int right_npl = x->right ? x->right->npl : -1;
      if (left_npl < right_npl) {
        std::swap(x->left, x->right);
      }
    }
    x->npl = x->right ? x->right->npl + 1 : 0;
    return x;
  }

  Node* root_;
  size_t size_;

 public:
  /**
   * @brief default constructor
   */
  priority_queue() : size_(0), root_(nullptr) {}

  /**
   * @brief copy constructor
   * @param other the priority_queue to be copied
   */
  priority_queue(const priority_queue& other) : size_(other.size_) {
    root_ = copytree(other.root_, nullptr);
    size_ = other.size_;
  }

  /**
   * @brief deconstructor
   */
  ~priority_queue() { clear(root_); }

  /**
   * @brief Assignment operator
   * @param other the priority_queue to be assigned from
   * @return a reference to this priority_queue after assignment
   */
  priority_queue& operator=(const priority_queue& other) {
    if (this == &other) {
      return *this;
    }
    Node* newNode = copytree(other.root_, nullptr);
    clear(root_);
    root_ = nullptr;
    root_ = newNode;
    size_ = other.size_;
    return *this;
  }

  /**
   * @brief get the top element of the priority queue.
   * @return a reference of the top element.
   * @throws container_is_empty if empty() returns true
   */
  const T& top() const {
    if (empty()) {
      throw container_is_empty();
    }
    return root_->data;
  }

  /**
   * @brief push new element to the priority queue.
   * @param e the element to be pushed
   */
  void push(const T& e) {
    Node* newNode = new Node(e);
    try {
      root_ = merge_heap(root_, newNode);
      size_++;
    } catch (...) {
      delete newNode;
      throw;
    }
  }

  /**
   * @brief delete the top element from the priority queue.
   * @throws container_is_empty if empty() returns true
   */
  void pop() {
    if (empty()) {
      throw container_is_empty();
    }
    try {
      Node* old_root = root_;
      Node* left_child = root_->left;
      Node* right_child = root_->right;
      if (left_child) {
        left_child->parent = nullptr;
      }
      if (right_child) {
        right_child->parent = nullptr;
      }
      Node* new_root = merge_heap(left_child, right_child);
      delete old_root;
      root_ = new_root;
      size_--;
    } catch (...) {
      root_->left->parent = root_;
      root_->right->parent = root_;
      throw;
    }
  }

  /**
   * @brief return the number of elements in the priority queue.
   * @return the number of elements.
   */
  size_t size() const { return size_; }

  /**
   * @brief check if the container is empty.
   * @return true if it is empty, false otherwise.
   */
  bool empty() const { return size_ == 0; }

  /**
   * @brief merge another priority_queue into this one.
   * The other priority_queue will be cleared after merging.
   * The complexity is at most O(logn).
   * @param other the priority_queue to be merged.
   */
  void merge(priority_queue& other) {
    try {
      Node* new_root = merge_heap(root_, other.root_);
      size_ += other.size_;
      other.size_ = 0;
      other.root_ = nullptr;
      root_ = new_root;
    } catch (...) {
      throw;
    }
  }
};

}  // namespace sjtu

#endif