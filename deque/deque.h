#pragma once

#include <iostream>

template <typename T>
class Deque {
  private:
    static const size_t kBlockSize_ = 32;
    T** tab_;
    size_t size_ = 0;
    size_t externalCap_ = 0;
    size_t externalFrontCap_ = 0;
    size_t externalBackCap_ = 0;
    size_t externalFront_ = 0;
    size_t blockFront_ = 0;
    size_t externalBack_ = 0;
    size_t blockBack_ = 0;

    void defaultConstruction() {
        externalCap_ = externalBackCap_ = 1;
        tab_ = new T*[1];
        tab_[0] = reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
    }

    void expandExternalCap() {
        if (externalCap_ == 0) {
            defaultConstruction();
            return;
        }
        T** newTab = new T*[externalCap_ * 3];
        for (size_t i = externalFrontCap_; i < externalBackCap_; ++i) {
            newTab[i + externalCap_] = tab_[i];
        }
        delete[] tab_;
        tab_ = newTab;
        externalFrontCap_ += externalCap_;
        externalBackCap_ += externalCap_;
        externalFront_ += externalCap_;
        externalBack_ += externalCap_;
        externalCap_ *= 3;
    }

    void addFrontBlock() {
        if (externalFrontCap_ == 0) {
            expandExternalCap();
        }
        tab_[--externalFrontCap_] =
            reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
    }

    void increaseFront() {
        if (blockFront_ == 0) {
            if (externalFront_ == externalFrontCap_) {
                addFrontBlock();
            }
            --externalFront_;
            blockFront_ = kBlockSize_ - 1;
        } else {
            --blockFront_;
        }
    }

    void shrinkFront() {
        if (++blockFront_ == kBlockSize_) {
            ++externalFront_;
            blockFront_ = 0;
        }
    }

    void addBackBlock() {
        if (externalBackCap_ == externalCap_) {
            expandExternalCap();
        }
        tab_[externalBackCap_++] =
            reinterpret_cast<T*>(new char[kBlockSize_ * sizeof(T)]);
    }

    void increaseBack() {
        if (++blockBack_ == kBlockSize_) {
            if (++externalBack_ == externalBackCap_) {
                addBackBlock();
            }
            blockBack_ = 0;
        }
    }

    void shrinkBack() {
        if (blockBack_ == 0) {
            --externalBack_;
            blockBack_ = kBlockSize_ - 1;
        } else {
            --blockBack_;
        }
    }

    std::pair<size_t, size_t> getPos(size_t index) const {
        if (blockFront_ + index < kBlockSize_) {
            return {externalFront_, blockFront_ + index};
        }
        index -= kBlockSize_ - blockFront_;
        return {externalFront_ + 1 + index / kBlockSize_, index % kBlockSize_};
    }

    size_t getIndex(size_t externalPos, size_t blockPos) const {
        return (externalPos - externalFront_) * kBlockSize_ + blockPos -
               blockFront_;
    }

    template <bool isConst>
    class BaseIterator {
      private:
        T** externalPtr_;
        T* blockPtr_;

        void updateBlockPtr() {
            if (externalPos == 0) {
                return;
            }
            blockPtr_ = externalPtr_[externalPos - 1];
        }

        void moveToFront() {
            if (blockPos == 0) {
                --externalPos;
                blockPos = kBlockSize_ - 1;
                updateBlockPtr();
            } else {
                --blockPos;
            }
        }

        void moveToBack() {
            if (++blockPos == kBlockSize_) {
                ++externalPos;
                blockPos = 0;
                updateBlockPtr();
            }
        }

        size_t getValue() const {
            return externalPos * kBlockSize_ + blockPos;
        }

      public:
        size_t externalPos;
        size_t blockPos;

        using value_type = std::conditional_t<isConst, const T, T>;
        using pointer = std::conditional_t<isConst, const T*, T*>;
        using reference = std::conditional_t<isConst, const T&, T&>;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = int;

        BaseIterator(T** dequeArr, size_t externalPos, size_t blockPos)
            : externalPtr_(dequeArr),
              externalPos(externalPos + 1),
              blockPos(blockPos) {
            updateBlockPtr();
        }

        BaseIterator(const BaseIterator<false>& other) {
            externalPtr_ = other.externalPtr_;
            blockPtr_ = other.blockPtr_;
            externalPos = other.externalPos;
            blockPos = other.blockPos;
        }

        BaseIterator& operator=(BaseIterator<false> copy) {
            std::swap(externalPtr_, copy.externalPtr_);
            std::swap(blockPtr_, copy.blockPtr_);
            std::swap(externalPos, copy.externalPos);
            std::swap(blockPos, copy.blockPos);
            return *this;
        }

        BaseIterator& operator++() {
            moveToBack();
            return *this;
        }

        BaseIterator operator++(int) {
            BaseIterator copy(*this);
            moveToBack();
            return copy;
        }

        BaseIterator& operator--() {
            moveToFront();
            return *this;
        }

        BaseIterator operator--(int) {
            BaseIterator copy(*this);
            moveToFront();
            return copy;
        }

        BaseIterator& operator+=(int number) {
            size_t newValue = getValue() + number;
            externalPos = newValue / kBlockSize_;
            blockPos = newValue % kBlockSize_;
            updateBlockPtr();
            return *this;
        }

        BaseIterator& operator-=(int number) {
            (*this) += -number;
            return *this;
        }

        BaseIterator operator+(int number) const {
            BaseIterator copy(*this);
            copy += number;
            return copy;
        }

        BaseIterator operator-(int number) const {
            BaseIterator copy(*this);
            copy -= number;
            return copy;
        }

        reference operator*() const {
            return *(blockPtr_ + blockPos);
        }

        pointer operator->() const {
            return blockPtr_ + blockPos;
        }

        difference_type operator-(const BaseIterator& other) const {
            difference_type result =
                static_cast<difference_type>(getValue()) -
                static_cast<difference_type>(other.getValue());
            return result;
        }

        bool operator==(const BaseIterator& other) const {
            return getValue() == other.getValue();
        }

        bool operator!=(const BaseIterator& other) const {
            return !(*this == other);
        }

        bool operator<(const BaseIterator& other) const {
            return getValue() < other.getValue();
        }

        bool operator>=(const BaseIterator& other) const {
            return !(*this < other);
        }

        bool operator>(const BaseIterator& other) const {
            return other < *this;
        }

        bool operator<=(const BaseIterator& other) const {
            return !(other > *this);
        }
    };

  public:
    void push_front(const T& elem) {
        increaseFront();
        try {
            new (tab_[externalFront_] + blockFront_) T(elem);
        } catch (...) {
            shrinkFront();
            throw;
        }
        ++size_;
    }

    void pop_front() {
        (tab_[externalFront_] + blockFront_)->~T();
        shrinkFront();
        --size_;
    }

    void push_back(const T& elem) {
        new (tab_[externalBack_] + blockBack_) T(elem);
        increaseBack();
        ++size_;
    }

    void pop_back() {
        shrinkBack();
        (tab_[externalBack_] + blockBack_)->~T();
        --size_;
    }

    size_t size() const {
        return size_;
    }

    T& operator[](size_t index) {
        auto [externalPos, blockPos] = getPos(index);
        return *(tab_[externalPos] + blockPos);
    }

    const T& operator[](size_t index) const {
        auto [externalPos, blockPos] = getPos(index);
        return *(tab_[externalPos] + blockPos);
    }

    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        auto [externalPos, blockPos] = getPos(index);
        return *(tab_[externalPos] + blockPos);
    }

    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        auto [externalPos, blockPos] = getPos(index);
        return *(tab_[externalPos] + blockPos);
    }

    void clear() {
        for (; size_ != 0; pop_front()) {}
    }

    Deque() {
        defaultConstruction();
    }

    ~Deque() {
        clear();
        for (size_t i = externalFrontCap_; i < externalBackCap_; ++i) {
            delete[] reinterpret_cast<char*>(tab_[i]);
        }
        delete[] tab_;
    }

    Deque(const Deque<T>& other) {
        defaultConstruction();
        try {
            for (size_t i = 0; i < other.size(); ++i) {
                push_back(other[i]);
            }
        } catch (...) {
            clear();
        }
    }

    Deque<T>& operator=(Deque<T> copy) {
        std::swap(tab_, copy.tab_);
        std::swap(size_, copy.size_);
        std::swap(externalCap_, copy.externalCap_);
        std::swap(externalFrontCap_, copy.externalFrontCap_);
        std::swap(externalBackCap_, copy.externalBackCap_);
        std::swap(externalFront_, copy.externalFront_);
        std::swap(blockFront_, copy.blockFront_);
        std::swap(externalBack_, copy.externalBack_);
        std::swap(blockBack_, copy.blockBack_);
        return *this;
    }

    Deque(size_t size) {
        defaultConstruction();
        if (std::is_default_constructible<T>::value) {
            try {
                for (size_t i = 0; i < size; ++i) {
                    new (tab_[externalBack_] + blockBack_) T();
                    increaseBack();
                    ++size_;
                }
            } catch (...) {
                clear();
            }
        }
    }

    Deque(size_t count, const T& elem) {
        defaultConstruction();
        try {
            for (size_t i = 0; i < count; ++i) {
                push_back(elem);
            }
        } catch (...) {
            clear();
        }
    }

    using iterator = BaseIterator<false>;
    using const_iterator = BaseIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(tab_, externalFront_, blockFront_);
    }

    iterator end() {
        return iterator(tab_, externalBack_, blockBack_);
    }

    const_iterator begin() const {
        return const_iterator(tab_, externalFront_, blockFront_);
    }

    const_iterator end() const {
        return const_iterator(tab_, externalBack_, blockBack_);
    }

    const_iterator cbegin() const {
        return const_iterator(tab_, externalFront_, blockFront_);
    }

    const_iterator cend() const {
        return const_iterator(tab_, externalBack_, blockBack_);
    }

    reverse_iterator rbegin() {
        return std::make_reverse_iterator(end());
    }

    reverse_iterator rend() {
        return std::make_reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return std::make_reverse_iterator(cend());
    }

    const_reverse_iterator rend() const {
        return std::make_reverse_iterator(cbegin());
    }

    const_reverse_iterator crbegin() const {
        return std::make_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const {
        return std::make_reverse_iterator(cbegin());
    }

    void insert(iterator iter, const T& elem) {
        size_t index = getIndex(iter.externalPos - 1, iter.blockPos);
        push_back(elem);
        for (size_t i = size_ - 1; i != index; --i) {
            std::swap((*this)[i], (*this)[i - 1]);
        }
    }

    void erase(iterator iter) {
        size_t index = getIndex(iter.externalPos - 1, iter.blockPos);
        for (; index + 1 < size_; ++index) {
            std::swap((*this)[index], (*this)[index + 1]);
        }
        pop_back();
    }
};
