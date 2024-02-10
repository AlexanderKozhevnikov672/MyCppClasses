#pragma once

#include <iostream>
#include <memory>
#include <type_traits>

template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;

struct BaseControlBlock {
    int sharedCount;
    int weakCount;

    BaseControlBlock(int sharedCount, int weakCount)
        : sharedCount(sharedCount), weakCount(weakCount) {}

    virtual void killObject() = 0;

    virtual void killBlock() = 0;

    virtual void* getObjectPtr() = 0;

    virtual ~BaseControlBlock() = default;
};

template <typename U, typename Deleter, typename Alloc>
struct ControlBlockRegular : BaseControlBlock {
    U* ptr;
    Deleter deleter;
    Alloc alloc;

    void killObject() override {
        deleter(ptr);
        ptr = nullptr;
    }

    void killBlock() override {
        using AllocTraits = std::allocator_traits<Alloc>;
        using AllocControlBlock = typename AllocTraits::template rebind_alloc<
            ControlBlockRegular<U, Deleter, Alloc>>;
        using BlockAllocTraits = typename AllocTraits::template rebind_traits<
            ControlBlockRegular<U, Deleter, Alloc>>;
        AllocControlBlock newAlloc = alloc;
        BlockAllocTraits::deallocate(newAlloc, this, 1);
    }

    void* getObjectPtr() override {
        return static_cast<void*>(ptr);
    }

    ControlBlockRegular(int sharedCount, int weakCount, U* ptr,
                        const Deleter& deleter, const Alloc& alloc)
        : BaseControlBlock(sharedCount, weakCount),
          ptr(ptr),
          deleter(deleter),
          alloc(alloc) {}
};

template <typename U, typename Alloc>
struct ControlBlockMakeShared : BaseControlBlock {
    U object;
    Alloc alloc;

    void killObject() override {
        using AllocTraits = std::allocator_traits<Alloc>;
        using AllocObject = typename AllocTraits::template rebind_alloc<U>;
        using ObjectAllocTraits =
            typename AllocTraits::template rebind_traits<U>;
        AllocObject newAlloc = alloc;
        ObjectAllocTraits::destroy(newAlloc, &object);
    }

    void killBlock() override {
        using AllocTraits = std::allocator_traits<Alloc>;
        using AllocControlBlock = typename AllocTraits::template rebind_alloc<
            ControlBlockMakeShared<U, Alloc>>;
        using BlockAllocTraits = typename AllocTraits::template rebind_traits<
            ControlBlockMakeShared<U, Alloc>>;
        AllocControlBlock newAlloc = alloc;
        BlockAllocTraits::deallocate(newAlloc, this, 1);
    }

    void* getObjectPtr() override {
        return static_cast<void*>(&object);
    }

    template <typename... Args>
    ControlBlockMakeShared(int sharedCount, int weakCount, const Alloc& alloc,
                           Args&&... args)
        : BaseControlBlock(sharedCount, weakCount),
          object(std::forward<Args>(args)...),
          alloc(alloc) {}
};

template <typename T>
class SharedPtr {
  private:
    BaseControlBlock* cb_;

    SharedPtr(WeakPtr<T> wptr) : cb_(wptr.cb_) {
        if (cb_ != nullptr) {
            ++cb_->sharedCount;
        }
    }

  public:
    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

    void swap(SharedPtr& other) {
        std::swap(cb_, other.cb_);
    }

    SharedPtr() {
        cb_ = nullptr;
    }

    template <typename Deleter = std::default_delete<T>,
              typename Alloc = std::allocator<T>>
    SharedPtr(T* ptr, Deleter deleter = Deleter(), Alloc alloc = Alloc()) {
        using AllocTraits = std::allocator_traits<Alloc>;
        using AllocControlBlock = typename AllocTraits::template rebind_alloc<
            ControlBlockRegular<T, Deleter, Alloc>>;
        using BlockAllocTraits = typename AllocTraits::template rebind_traits<
            ControlBlockRegular<T, Deleter, Alloc>>;
        AllocControlBlock newAlloc = alloc;
        auto cb = BlockAllocTraits::allocate(newAlloc, 1);
        new (cb)
            ControlBlockRegular<T, Deleter, Alloc>(1, 0, ptr, deleter, alloc);
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            cb->ptr->wptr = *this;
        }
        cb_ = cb;
    }

    template <typename U, typename Deleter = std::default_delete<T>,
              typename Alloc = std::allocator<T>>
    SharedPtr(U* ptr, Deleter deleter = Deleter(), Alloc alloc = Alloc()) {
        using AllocTraits = std::allocator_traits<Alloc>;
        using AllocControlBlock = typename AllocTraits::template rebind_alloc<
            ControlBlockRegular<U, Deleter, Alloc>>;
        using BlockAllocTraits = typename AllocTraits::template rebind_traits<
            ControlBlockRegular<U, Deleter, Alloc>>;
        AllocControlBlock newAlloc = alloc;
        auto cb = BlockAllocTraits::allocate(newAlloc, 1);
        new (cb)
            ControlBlockRegular<U, Deleter, Alloc>(1, 0, ptr, deleter, alloc);
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            cb->ptr->wptr = *this;
        }
        cb_ = cb;
    }

    SharedPtr(const SharedPtr<T>& other) : cb_(other.cb_) {
        if (cb_ != nullptr) {
            ++cb_->sharedCount;
        }
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : cb_(other.cb_) {
        if (cb_ != nullptr) {
            ++cb_->sharedCount;
        }
    }

    SharedPtr(SharedPtr<T>&& other) : cb_(other.cb_) {
        other.cb_ = nullptr;
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) : cb_(other.cb_) {
        other.cb_ = nullptr;
    }

    SharedPtr& operator=(const SharedPtr<T>& other) {
        if (this == &other) {
            return *this;
        }
        SharedPtr<T> copy(other);
        swap(copy);
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        SharedPtr<T> copy(other);
        swap(copy);
        return *this;
    }

    SharedPtr& operator=(SharedPtr<T>&& other) {
        SharedPtr<T> copy(std::move(other));
        swap(copy);
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        SharedPtr<T> copy(std::move(other));
        swap(copy);
        return *this;
    }

    ~SharedPtr() {
        if (cb_ == nullptr) {
            return;
        }
        if (--cb_->sharedCount == 0) {
            cb_->killObject();
            if (cb_->weakCount == 0) {
                cb_->killBlock();
                cb_ = nullptr;
            }
        }
    }

    int use_count() const noexcept {
        if (cb_ == nullptr) {
            return 0;
        }
        return cb_->sharedCount;
    }

    void reset(T* ptr) {
        *this = SharedPtr<T>(ptr);
    }

    template <typename U>
    void reset(U* ptr) {
        *this = SharedPtr<T>(ptr);
    }

    void reset() {
        *this = SharedPtr<T>();
    }

    T* operator->() const {
        if (cb_ == nullptr) {
            return nullptr;
        }
        return static_cast<T*>(cb_->getObjectPtr());
    }

    T& operator*() const {
        return *(static_cast<T*>(cb_->getObjectPtr()));
    }

    T* get() const {
        if (cb_ == nullptr) {
            return nullptr;
        }
        return static_cast<T*>(cb_->getObjectPtr());
    }

    template <typename Alloc, typename... Args>
    SharedPtr(const Alloc& alloc, Args&&... args) {
        using AllocTraits = std::allocator_traits<Alloc>;
        using AllocControlBlock = typename AllocTraits::template rebind_alloc<
            ControlBlockMakeShared<T, Alloc>>;
        using BlockAllocTraits = typename AllocTraits::template rebind_traits<
            ControlBlockMakeShared<T, Alloc>>;
        AllocControlBlock newAlloc = alloc;
        auto cb = BlockAllocTraits::allocate(newAlloc, 1);
        BlockAllocTraits::construct(newAlloc, cb, 1, 0, alloc,
                                    std::forward<Args>(args)...);
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            cb->object.wptr = *this;
        }
        cb_ = cb;
    }
};

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
    return SharedPtr<T>(alloc, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return allocateShared<T, std::allocator<T>>(std::allocator<T>(),
                                                std::forward<Args>(args)...);
}

template <typename T>
class WeakPtr {
  private:
    BaseControlBlock* cb_;

  public:
    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

    void swap(WeakPtr& other) {
        std::swap(cb_, other.cb_);
    }

    WeakPtr() {
        cb_ = nullptr;
    }

    WeakPtr(const SharedPtr<T>& sptr) : cb_(sptr.cb_) {
        if (cb_ != nullptr) {
            ++cb_->weakCount;
        }
    }

    template <typename U>
    WeakPtr(const SharedPtr<U>& sptr) : cb_(sptr.cb_) {
        if (cb_ != nullptr) {
            ++cb_->weakCount;
        }
    }

    WeakPtr(const WeakPtr<T>& other) : cb_(other.cb_) {
        if (cb_ != nullptr) {
            ++cb_->weakCount;
        }
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& other) : cb_(other.cb_) {
        if (cb_ != nullptr) {
            ++cb_->weakCount;
        }
    }

    WeakPtr(WeakPtr<T>&& other) : cb_(other.cb_) {
        other.cb_ = nullptr;
    }

    template <typename U>
    WeakPtr(WeakPtr<U>&& other) : cb_(other.cb_) {
        other.cb_ = nullptr;
    }

    WeakPtr& operator=(const WeakPtr<T>& other) {
        WeakPtr<T> copy(other);
        swab(copy);
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(const WeakPtr<U>& other) {
        WeakPtr<T> copy(other);
        swab(copy);
        return *this;
    }

    WeakPtr& operator=(WeakPtr<T>&& other) {
        WeakPtr<T> copy(std::move(other));
        swap(copy);
        return *this;
    }

    template <typename U>
    WeakPtr& operator=(WeakPtr<U>&& other) {
        WeakPtr<T> copy(std::move(other));
        swap(copy);
        return *this;
    }

    bool expired() const noexcept {
        if (cb_ == nullptr) {
            return true;
        }
        return cb_->sharedCount == 0;
    }

    SharedPtr<T> lock() const noexcept {
        return expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
    }

    ~WeakPtr() {
        if (cb_ == nullptr) {
            return;
        }
        if (--cb_->weakCount == 0 && cb_->sharedCount == 0) {
            cb_->killBlock();
            cb_ = nullptr;
        }
    }

    int use_count() const noexcept {
        if (cb_ == nullptr) {
            return 0;
        }
        return cb_->sharedCount;
    }
};

template <typename T>
class EnableSharedFromThis {
  private:
    WeakPtr<T> wprt_;

  public:
    SharedPtr<T> shared_from_this() const noexcept {
        return wprt_.lock();
    }
};
