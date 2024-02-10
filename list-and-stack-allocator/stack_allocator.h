#pragma once

template <size_t N>
class alignas(max_align_t) StackStorage {
  private:
    char pool_[N];
    size_t placeGiven_ = 0;

  public:
    StackStorage() = default;

    ~StackStorage() = default;

    StackStorage& operator=(const StackStorage& other) = default;

    char* getFreePlace(size_t size, size_t alignment) {
        placeGiven_ = ((placeGiven_ + alignment - 1) / alignment) * alignment;
        char* result = pool_ + placeGiven_;
        placeGiven_ += size;
        return result;
    }
};

template <typename T, size_t N>
class StackAllocator {
  public:
    StackStorage<N>* storagePtr;

    ~StackAllocator() = default;

    StackAllocator(StackStorage<N>& storage)
        : storagePtr(&storage) {}

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other)
        : storagePtr(other.storagePtr) {}

    template <typename U>
    StackAllocator& operator=(const StackAllocator<U, N>& other) {
        storagePtr = other.storagePtr;
        return *this;
    }

    T* allocate(size_t count) {
        return reinterpret_cast<T*>(storagePtr->getFreePlace(count * sizeof(T), alignof(T)));
    }

    void deallocate(T* ptr, size_t /*unused*/) {
        std::ignore = ptr;
    }

    using value_type = T;

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    template <typename U, size_t M>
    bool operator==(const StackAllocator<U, M>& other) {
        return storagePtr == other.storagePtr;
    }

    template <typename U, size_t M>
    bool operator!=(const StackAllocator<U, M>& other) {
        return !((*this) == other);
    }
};
