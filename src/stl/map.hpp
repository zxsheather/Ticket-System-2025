// acmoj: 2671

/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <cstddef>
#include <functional>

#include "exceptions.hpp"
#include "utility.hpp"

namespace sjtu {

template <class Key, class T, class Compare = std::less<Key> >
class map {
 public:
  typedef pair<const Key, T> value_type;

 private:
  struct Node {
    value_type data;
    Node *left, *right, *parent;
    int height;
    Node(const value_type& data)
        : data(data),
          left(nullptr),
          right(nullptr),
          parent(nullptr),
          height(1) {}
    Node(const value_type& data, Node* l, Node* r, Node* p)
        : data(data), left(l), right(r), parent(p), height(1) {}
  };

  Node* root_;
  size_t size_{};
  Compare cmp{};

  Node* copytree(Node* src, Node* parent) {
    if (!src) {
      return nullptr;
    }
    Node* node = new Node(src->data);
    node->parent = parent;
    node->height = src->height;
    node->left = copytree(src->left, node);
    node->right = copytree(src->right, node);
    return node;
  }
  void clear(Node*& node) {
    if (node) {
      clear(node->left);
      clear(node->right);
      delete node;
      node = nullptr;
    }
  }
  int height(Node* node) { return node ? node->height : 0; }
  int balanceFactor(Node* node) {
    if (!node) return 0;
    return height(node->left) - height(node->right);
  }
  void updateHeight(Node* node) {
    if (node) {
      node->height = 1 + std::max(height(node->left), height(node->right));
    }
  }
  void leftRotate(Node*& node) {
    if (!node || !node->right) return;
    Node* node_ = node;
    Node* subR = node_->right;
    Node* subRL = subR->left;
    Node* parent = node_->parent;
    node_->right = subRL;
    if (subRL) subRL->parent = node_;
    subR->left = node_;
    node_->parent = subR;
    if (parent) {
      if (parent->left == node_) {
        parent->left = subR;
      } else {
        parent->right = subR;
      }
      subR->parent = parent;
    } else {
      subR->parent = nullptr;
    }
    updateHeight(node_);
    updateHeight(subR);
    node = subR;
  }
  void rightRotate(Node*& node) {
    if (!node || !node->left) return;
    Node* node_ = node;
    Node* subL = node_->left;
    Node* subLR = subL->right;
    Node* parent = node_->parent;
    node_->left = subLR;
    if (subLR) subLR->parent = node_;
    subL->right = node_;
    node_->parent = subL;
    if (parent) {
      if (parent->left == node_) {
        parent->left = subL;
      } else {
        parent->right = subL;
      }
      subL->parent = parent;
    } else {
      subL->parent = nullptr;
    }
    updateHeight(node_);
    updateHeight(subL);
    node = subL;
  }
  pair<Node*, bool> insert(Node*& node, const Key& key, const T& value) {
    if (!node) {
      node = new Node({key, value});
      size_++;
      // node->parent = parent;
      node->height = 1;
      return {node, true};
    }
    if (cmp(key, node->data.first)) {
      auto result = insert(node->left, key, value);
      if (result.second) {
        node->left->parent = node;
        if (balanceFactor(node) == 2) {
          if (cmp(key, node->left->data.first)) {
            rightRotate(node);
            return {result.first, true};
          } else {
            leftRotate(node->left);
            rightRotate(node);
            return {result.first, true};
          }
        } else {
          updateHeight(node);
          return {result.first, true};
        }
      } else {
        updateHeight(node);
        return {result.first, false};
      }
    } else if (cmp(node->data.first, key)) {
      auto result = insert(node->right, key, value);
      if (result.second) {
        node->right->parent = node;
        if (balanceFactor(node) == -2) {
          if (cmp(node->right->data.first, key)) {
            leftRotate(node);

            return {result.first, true};
          } else {
            rightRotate(node->right);
            leftRotate(node);

            return {result.first, true};
          }
        } else {
          updateHeight(node);
          return {result.first, true};
        }
      } else {
        updateHeight(node);
        return {result.first, false};
      }
    } else {
      return {node, false};
    }
  }
  Node* find(Node* node, const Key& key) const {
    if (!node) return nullptr;
    if (cmp(key, node->data.first)) {
      return find(node->left, key);
    } else if (cmp(node->data.first, key)) {
      return find(node->right, key);
    } else {
      return node;
    }
  }

  Node* swapNode(Node* dst, Node* src) {
    Node* dstParent = dst->parent;
    Node* srcParent = src->parent;
    Node* dstLeft = dst->left;
    Node* dstRight = dst->right;
    Node* srcLeft = src->left;
    Node* srcRight = src->right;
    if (src->parent != dst) {
      dst->parent = srcParent;
      dst->left = srcLeft;
      dst->right = srcRight;
      src->parent = dstParent;
      src->left = dstLeft;
      src->right = dstRight;
      if (srcParent) {
        if (srcParent->left == src) {
          srcParent->left = dst;
        } else {
          srcParent->right = dst;
        }
      }
      if (dstParent) {
        if (dstParent->left == dst) {
          dstParent->left = src;
        } else {
          dstParent->right = src;
        }
      }
      if (srcLeft) srcLeft->parent = dst;
      if (srcRight) srcRight->parent = dst;
      if (dstLeft) dstLeft->parent = src;
      if (dstRight) dstRight->parent = src;
    } else {
      if (dst->right == src) {
        dst->right = srcRight;
        dst->parent = src;
        dst->left = srcLeft;
        src->left = dstLeft;
        src->right = dst;
        src->parent = dstParent;
        if (srcRight) srcRight->parent = dst;
        if (srcLeft) srcLeft->parent = dst;
        if (dstLeft) dstLeft->parent = src;
        if (dstParent) {
          if (dstParent->left == dst) {
            dstParent->left = src;
          } else {
            dstParent->right = src;
          }
        }
      }
    }
    std::swap(dst->height, src->height);
    return src;
  }

  pair<bool, bool> erase(Node*& node, const Key& key) {
    if (!node) return {true, false};
    if (cmp(key, node->data.first)) {
      auto result = erase(node->left, key);
      if (node->left) node->left->parent = node;
      if (result.first) {
        return {true, result.second};
      } else {
        return {adjust(node, 0), result.second};
      }
    } else if (cmp(node->data.first, key)) {
      auto result = erase(node->right, key);
      if (node->right) node->right->parent = node;
      if (result.first) {
        return {true, result.second};
      } else {
        return {adjust(node, 1), result.second};
      }
    } else {
      if (!node->left && !node->right) {
        Node* node_ = node;
        if (node->parent) {
          if (node->parent->left == node) {
            node->parent->left = nullptr;
          } else {
            node->parent->right = nullptr;
          }
        }
        delete node_;
        node = nullptr;
        return {false, true};
      } else if (!node->left) {
        Node* node_ = node;
        Node* temp = node->right;
        Node* parent = node->parent;
        if (parent) {
          if (parent->left == node) {
            parent->left = temp;
          } else {
            parent->right = temp;
          }
          temp->parent = parent;
        } else {
          temp->parent = nullptr;
        }
        delete node_;
        node = temp;
        return {false, true};
      } else if (!node->right) {
        Node* node_ = node;
        Node* temp = node->left;
        Node* parent = node->parent;
        if (parent) {
          if (parent->left == node) {
            parent->left = temp;
          } else {
            parent->right = temp;
          }
          temp->parent = parent;
        } else {
          temp->parent = nullptr;
        }
        delete node_;
        node = temp;
        return {false, true};
      } else {
        Node* node_ = node;
        Node* temp = node_->right;
        while (temp->left) {
          temp = temp->left;
        }
        // node->data.~value_type();
        // new (&node->data) value_type(temp->data);
        // temp->height = node_->height;
        // node_->height = 1;
        // if (node_->right == temp) {
        //   if (node_->parent) {
        //     if (node_->parent->left == node_) {
        //       node_->parent->left = temp;
        //     } else {
        //       node_->parent->right = temp;
        //     }
        //   }
        //   node_->right = temp->right;
        //   if (temp->right) temp->right->parent = node_;
        //   node_->parent = temp;
        //   temp->right = node_;
        //   temp->left = node_->left;
        //   if (node_->left) node_->left->parent = temp;
        //   node_->left = nullptr;
        //   node = temp;
        // } else {
        //   if (node_->parent) {
        //     if (node_->parent->left == node_) {
        //       node_->parent->left = temp;
        //     } else {
        //       node_->parent->right = temp;
        //     }
        //   }
        //   Node *nodeRight = node_->right;
        //   node_->right = temp->right;
        //   if (nodeRight) nodeRight->parent = temp;
        //   if (temp->right) temp->right->parent = node_;
        //   temp->right = nodeRight;
        //   Node *parent = temp->parent;
        //   temp->parent = node_->parent;
        //   parent->left = node_;
        //   node_->parent = parent;
        //   temp->left = node_->left;
        //   if (node_->left) node_->left->parent = temp;
        //   node_->left = nullptr;
        //   node = temp;
        // }
        node = swapNode(node, temp);
        auto result = erase(node->right, key);
        if (node->right) node->right->parent = node;
        if (result.first) {
          return {true, result.second};
        } else {
          return {adjust(node, 1), result.second};
        }
      }
    }
  }

  bool adjust(Node*& node, int subTree) {
    if (subTree) {
      if (balanceFactor(node) == 1) {
        return true;
      } else if (balanceFactor(node) == 0) {
        --node->height;
        return false;
      } else if (height(node->left->left) >= height(node->left->right)) {
        rightRotate(node);
        if (balanceFactor(node) == 0) {
          return false;
        } else {
          return true;
        }
      } else {
        leftRotate(node->left);
        rightRotate(node);
        return false;
      }
    } else {
      if (balanceFactor(node) == -1) {
        return true;
      } else if (balanceFactor(node) == 0) {
        --node->height;
        return false;
      } else if (height(node->right->right) >= height(node->right->left)) {
        leftRotate(node);
        if (balanceFactor(node) == 0) {
          return false;
        } else {
          return true;
        }
      } else {
        rightRotate(node->right);
        leftRotate(node);
        return false;
      }
    }
  }

 public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */

  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */
  class const_iterator;
  class iterator {
   public:
    Node* node{};
    map* container{};

   public:
    iterator() {}

    iterator(const iterator& other)
        : node(other.node), container(other.container) {}

    iterator(Node* node, map* container) : node(node), container(container) {}

    /**
     * TODO iter++
     */
    iterator operator++(int) {
      iterator temp = *this;
      if (node) {
        if (node->right) {
          node = node->right;
          while (node->left) {
            node = node->left;
          }
        } else {
          Node* parent = node->parent;
          while (parent && parent->right == node) {
            node = parent;
            parent = parent->parent;
          }
          node = parent;
        }
      } else {
        throw invalid_iterator();
      }
      return temp;
    }

    /**
     * TODO ++iter
     */
    iterator& operator++() {
      if (node) {
        if (node->right) {
          node = node->right;
          while (node->left) {
            node = node->left;
          }
        } else {
          Node* parent = node->parent;
          while (parent && parent->right == node) {
            node = parent;
            parent = parent->parent;
          }
          node = parent;
        }
      } else {
        throw invalid_iterator();
      }
      return *this;
    }

    /**
     * TODO iter--
     */
    iterator operator--(int) {
      iterator temp = *this;
      if (node) {
        if (node->left) {
          node = node->left;
          while (node->right) {
            node = node->right;
          }
        } else {
          Node* parent = node->parent;
          if (parent == nullptr) {
            throw invalid_iterator();
          }
          while (parent && parent->left == node) {
            node = parent;
            parent = parent->parent;
          }
          node = parent;
        }
      } else {
        if (!container || !container->root_) {
          throw invalid_iterator();
        }
        node = container->root_;
        while (node && node->right) {
          node = node->right;
        }
      }
      return temp;
    }

    /**
     * TODO --iter
     */
    iterator& operator--() {
      if (node) {
        if (node->left) {
          node = node->left;
          while (node->right) {
            node = node->right;
          }
        } else {
          if (node->parent == nullptr) {
            throw invalid_iterator();
          }
          Node* parent = node->parent;
          while (parent && parent->left == node) {
            node = parent;
            parent = parent->parent;
          }
          node = parent;
        }
      } else {
        if (!container || !container->root_) {
          throw invalid_iterator();
        }
        node = container->root_;
        while (node && node->right) {
          node = node->right;
        }
      }
      return *this;
    }

    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory).
     */
    value_type& operator*() const {
      if (!node) {
        throw invalid_iterator();
      }
      return node->data;
    }

    bool operator==(const iterator& rhs) const {
      return node == rhs.node && container == rhs.container;
    }

    bool operator==(const const_iterator& rhs) const {
      return node == rhs.node && container == rhs.container;
    }

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator& rhs) const {
      return node != rhs.node || container != rhs.container;
    }

    bool operator!=(const const_iterator& rhs) const {
      return node != rhs.node || container != rhs.container;
    }

    /**
     * for the support of it->first.
     * See
     * <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/>
     * for help.
     */
    value_type* operator->() const noexcept {
      if (!node) {
        throw invalid_iterator();
      }
      return &node->data;
    }
  };
  class const_iterator {
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
   public:
    Node* node{};
    const map* container{};

   public:
    const_iterator() {}

    const_iterator(const const_iterator& other)
        : node(other.node), container(other.container) {}

    const_iterator(const iterator& other)
        : node(other.node), container(other.container) {}
    const_iterator(Node* node, const map* container)
        : node(node), container(container) {}

    const_iterator& operator=(const const_iterator& other) {
      if (this != &other) {
        node = other.node;
        container = other.container;
      }
      return *this;
    }
    const_iterator& operator=(const iterator& other) {
      node = other.node;
      container = other.container;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator temp = *this;
      if (node) {
        if (node->right) {
          node = node->right;
          while (node->left) {
            node = node->left;
          }
        } else {
          Node* parent = node->parent;
          while (parent && parent->right == node) {
            node = parent;
            parent = parent->parent;
          }
          node = parent;
        }
      } else {
        throw invalid_iterator();
      }
      return temp;
    }
    const_iterator& operator++() {
      if (node) {
        if (node->right) {
          node = node->right;
          while (node->left) {
            node = node->left;
          }
        } else {
          Node* parent = node->parent;
          while (parent && parent->right == node) {
            node = parent;
            parent = parent->parent;
          }
          node = parent;
        }
      } else {
        throw invalid_iterator();
      }
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator temp = *this;
      if (node) {
        if (node->left) {
          node = node->left;
          while (node->right) {
            node = node->right;
          }
        } else {
          if (node->parent == nullptr) {
            throw invalid_iterator();
          }
          Node* parent = node->parent;
          while (parent && parent->left == node) {
            node = parent;
            parent = parent->parent;
          }
          node = parent;
        }
      } else {
        if (!container || !container->root_) {
          throw invalid_iterator();
        }
        node = container->root_;
        while (node && node->right) {
          node = node->right;
        }
      }
      return temp;
    }
    const_iterator& operator--() {
      if (node) {
        if (node->left) {
          node = node->left;
          while (node->right) {
            node = node->right;
          }
        } else {
          if (node->parent == nullptr) {
            throw invalid_iterator();
          }
          Node* parent = node->parent;
          while (parent && parent->left == node) {
            node = parent;
            parent = parent->parent;
          }
          node = parent;
        }
      } else {
        if (!container || !container->root_) {
          throw invalid_iterator();
        }
        node = container->root_;
        while (node && node->right) {
          node = node->right;
        }
      }
      return *this;
    }
    value_type& operator*() const {
      if (!node) {
        throw invalid_iterator();
      }
      return node->data;
    }
    bool operator==(const iterator& rhs) const {
      return node == rhs.node && container == rhs.container;
    }
    bool operator==(const const_iterator& rhs) const {
      return node == rhs.node && container == rhs.container;
    }
    bool operator!=(const iterator& rhs) const {
      return node != rhs.node || container != rhs.container;
    }
    bool operator!=(const const_iterator& rhs) const {
      return node != rhs.node || container != rhs.container;
    }
    value_type* operator->() const noexcept {
      if (!node) {
        throw invalid_iterator();
      }
      return &node->data;
    }
  };

  /**
   * TODO two constructors
   */
  map() : root_(nullptr), size_(0) {}
  map(const Compare& cmp) : root_(nullptr), size_(0), cmp(cmp) {}

  map(const map& other) : root_(nullptr), size_(other.size_), cmp(other.cmp) {
    root_ = copytree(other.root_, nullptr);
  }

  map& operator=(const map& other) {
    if (this != &other) {
      clear(root_);
      size_ = other.size_;
      cmp = other.cmp;
      root_ = copytree(other.root_, nullptr);
    }
    return *this;
  }

  ~map() { clear(root_); }

  T& at(const Key& key) {
    Node* node = find(root_, key);
    if (!node) {
      throw index_out_of_bound();
    }
    return node->data.second;
  }

  const T& at(const Key& key) const {
    Node* node = find(root_, key);
    if (!node) {
      throw index_out_of_bound();
    }
    return node->data.second;
  }

  T& operator[](const Key& key) {
    auto result = insert(root_, key, T());
    return result.first->data.second;
  }

  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  const T& operator[](const Key& key) const {
    Node* node = find(root_, key);
    if (!node) {
      throw index_out_of_bound();
    }
    return node->data.second;
  }

  /**
   * return a iterator to the beginning
   */
  iterator begin() {
    Node* node = root_;
    while (node && node->left) {
      node = node->left;
    }
    return iterator(node, this);
  }

  const_iterator cbegin() const {
    Node* node = root_;
    while (node && node->left) {
      node = node->left;
    }
    return const_iterator(node, this);
  }

  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() { return iterator(nullptr, this); }

  const_iterator cend() const { return const_iterator(nullptr, this); }

  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  bool empty() const { return size_ == 0; }

  /**
   * returns the number of elements.
   */
  size_t size() const { return size_; }

  /**
   * clears the contents
   */
  void clear() {
    clear(root_);
    root_ = nullptr;
    size_ = 0;
  }

  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the
   * insertion), the second one is true if insert successfully, or false.
   */
  pair<iterator, bool> insert(const value_type& value) {
    auto result = insert(root_, value.first, value.second);
    return {iterator(result.first, this), result.second};
  }

  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points an
   * element out of this)
   */
  void erase(iterator pos) {
    if (pos.node == nullptr || pos.container != this) {
      throw invalid_iterator();
    }
    Key key = pos.node->data.first;
    auto result = erase(root_, key);
    if (!result.second) {
      throw invalid_iterator();
    } else {
      size_--;
    }
  }

  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key& key) const {
    Node* node = find(root_, key);
    if (node) {
      return 1;
    } else {
      return 0;
    }
  }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is
   * returned.
   */
  iterator find(const Key& key) {
    Node* node = find(root_, key);
    if (node) {
      return iterator(node, this);
    } else {
      return end();
    }
  }

  const_iterator find(const Key& key) const {
    Node* node = find(root_, key);
    if (node) {
      return const_iterator(node, this);
    } else {
      return cend();
    }
  }
};

}  // namespace sjtu

#endif
