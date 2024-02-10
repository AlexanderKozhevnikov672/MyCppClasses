#pragma once

template <typename T, typename Allocator = std::allocator<T>>
class List {
  private:
    struct BaseNode {
        BaseNode* next = this;
        BaseNode* prev = this;

        BaseNode() = default;

        ~BaseNode() = default;

        void swap(BaseNode& other) {
            prev->next = &other;
            next->prev = &other;
            other.next->prev = this;
            other.prev->next = this;
            std::swap(next, other.next);
            std::swap(prev, other.prev);
        }
    };

    struct Node : BaseNode {
        T value;

        Node(const T& val)
            : value(val) {}

        Node() = default;

        ~Node() = default;
    };

    using AllocTraits = std::allocator_traits<Allocator>;
    using NodeAlloc = typename AllocTraits::template rebind_alloc<Node>;
    using NodeAllocTraits = typename AllocTraits::template rebind_traits<Node>;

    size_t size_ = 0;
    BaseNode fakeNode_;
    Allocator allocator_;
    NodeAlloc nodeAllocator_ = allocator_;

    void insert(BaseNode* oldNodePtr, BaseNode* newNodePtr) {
        newNodePtr->prev = oldNodePtr->prev;
        newNodePtr->next = oldNodePtr;
        oldNodePtr->prev->next = newNodePtr;
        oldNodePtr->prev = newNodePtr;
    }

    void erase(BaseNode* nodePtr) {
        nodePtr->prev->next = nodePtr->next;
        nodePtr->next->prev = nodePtr->prev;
    }

    Node* createNode(const T& elem) {
        Node* newNodePtr = NodeAllocTraits::allocate(nodeAllocator_, 1);
        try {
            NodeAllocTraits::construct(nodeAllocator_, newNodePtr, elem);
        } catch (...) {
            NodeAllocTraits::deallocate(nodeAllocator_, newNodePtr, 1);
            throw;
        }
        ++size_;
        return newNodePtr;
    }

    void destroyNode(Node* nodePtr) {
        NodeAllocTraits::destroy(nodeAllocator_, nodePtr);
        NodeAllocTraits::deallocate(nodeAllocator_, nodePtr, 1);
        --size_;
    }

    Node* createNode() {
        Node* newNodePtr = NodeAllocTraits::allocate(nodeAllocator_, 1);
        try {
            NodeAllocTraits::construct(nodeAllocator_, newNodePtr);
        } catch (...) {
            NodeAllocTraits::deallocate(nodeAllocator_, newNodePtr, 1);
            throw;
        }
        ++size_;
        return newNodePtr;
    }

    void pushEmpty() {
        insert(&fakeNode_, static_cast<BaseNode*>(createNode()));
    }

    void constructList(size_t count) {
        if (std::is_default_constructible<T>::value) {
            try {
                for (size_t i = 0; i < count; ++i) {
                    pushEmpty();
                }
            } catch (...) {
                clear();
                throw;
            }
        }
    }

    void constructList(size_t count, const T& elem) {
        try {
            for (size_t i = 0; i < count; ++i) {
                push_back(elem);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    template <bool isConst>
    class BaseIterator {
      private:
        using BaseNodePtr = std::conditional_t<isConst, const BaseNode*, BaseNode*>;
        using NodePtr = std::conditional_t<isConst, const Node*, Node*>;

        BaseNodePtr nodePtr_;

      public:
        using value_type = std::conditional_t<isConst, const T, T>;
        using pointer = std::conditional_t<isConst, const T*, T*>;
        using reference = std::conditional_t<isConst, const T&, T&>;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;

        BaseIterator(BaseNodePtr nodePtr)
            : nodePtr_(nodePtr) {}

        BaseIterator(const BaseIterator& other)
            : nodePtr_(other.nodePtr_) {}

        BaseIterator& operator=(const BaseIterator& other) {
            nodePtr_ = other.nodePtr_;
            return *this;
        }

        operator BaseIterator<true>() const {
            BaseIterator<true> result(nodePtr_);
            return result;
        }

        BaseIterator& operator++() {
            nodePtr_ = nodePtr_->next;
            return *this;
        }

        BaseIterator operator++(int) {
            BaseIterator copy = *this;
            ++(*this);
            return copy;
        }

        BaseIterator& operator--() {
            nodePtr_ = nodePtr_->prev;
            return *this;
        }

        BaseIterator operator--(int) {
            BaseIterator copy = *this;
            --(*this);
            return copy;
        }

        reference operator*() const {
            return static_cast<NodePtr>(nodePtr_)->value;
        }

        pointer operator->() const {
            return &(static_cast<NodePtr>(nodePtr_)->value);
        }

        bool operator==(const BaseIterator& other) const {
            return nodePtr_ == other.nodePtr_;
        }

        bool operator!=(const BaseIterator& other) const {
            return !((*this) == other);
        }

        BaseNodePtr getNodePtr() const {
            return nodePtr_;
        }
    };

  public:
    List() = default;

    void push_back(const T& elem) {
        insert(&fakeNode_, static_cast<BaseNode*>(createNode(elem)));
    }

    void push_front(const T& elem) {
        insert(fakeNode_.next, static_cast<BaseNode*>(createNode(elem)));
    }

    void pop_back() {
        BaseNode* nodePtr = fakeNode_.prev;
        erase(nodePtr);
        destroyNode(static_cast<Node*>(nodePtr));
    }

    void pop_front() {
        BaseNode* nodePtr = fakeNode_.next;
        erase(nodePtr);
        destroyNode(static_cast<Node*>(nodePtr));
    }

    void clear() {
        while (size_ != 0) {
            pop_back();
        }
    }

    ~List() {
        clear();
    }

    List(size_t count) {
        constructList(count);
    }

    List(size_t count, const T& elem) {
        constructList(count, elem);
    }

    List(const Allocator& allocator)
        : allocator_(allocator) {}

    List(size_t count, const Allocator& allocator)
        : allocator_(allocator) {
        constructList(count);
    }

    List(size_t count, const T& elem, const Allocator& allocator)
        : allocator_(allocator) {
        constructList(count, elem);
    }

    Allocator get_allocator() const {
        return allocator_;
    }

    size_t size() const {
        return size_;
    }

    using iterator = BaseIterator<false>;
    using const_iterator = BaseIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(fakeNode_.next);
    }

    iterator end() {
        return iterator(&fakeNode_);
    }

    const_iterator begin() const {
        return const_iterator(fakeNode_.next);
    }

    const_iterator end() const {
        return const_iterator(&fakeNode_);
    }

    const_iterator cbegin() const {
        return const_iterator(fakeNode_.next);
    }

    const_iterator cend() const {
        return const_iterator(&fakeNode_);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(&fakeNode_);
    }

    reverse_iterator rend() {
        return reverse_iterator(fakeNode_.next);
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(&fakeNode_);
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(fakeNode_.next);
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(&fakeNode_);
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(fakeNode_.next);
    }

    iterator insert(const_iterator iter, const T& elem) {
        BaseNode* newNodePtr = static_cast<BaseNode*>(createNode(elem));
        insert(const_cast<BaseNode*>(iter.getNodePtr()), newNodePtr);  // NOLINT
        return iterator(newNodePtr);
    }

    void erase(const_iterator iter) {
        BaseNode* nodePtr = const_cast<BaseNode*>(iter.getNodePtr());  // NOLINT
        erase(nodePtr);
        destroyNode(static_cast<Node*>(nodePtr));
    }

    List(const List& other)
        : allocator_(AllocTraits::select_on_container_copy_construction(other.allocator_)) {
        try {
            for (const_iterator iter = other.cbegin(); iter != other.cend(); ++iter) {
                push_back(*iter);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List& operator=(const List& other) {
        List copy(other);
        std::swap(size_, copy.size_);
        fakeNode_.swap(copy.fakeNode_);
        if (AllocTraits::propagate_on_container_copy_assignment::value) {
            allocator_ = other.allocator_;
            nodeAllocator_ = other.nodeAllocator_;
        }
        return *this;
    }
};
