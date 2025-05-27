#pragma once

#include <array>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <initializer_list>
#include <cassert>
#include <span>

namespace edge::utl {
    template <typename _Ty, std::size_t _Capacity = 8>
	class fixed_vector {
    public:
        using value_type = _Ty;
        using size_type = std::size_t;
        using reference = _Ty&;
        using const_reference = const _Ty&;
        using iterator = _Ty*;
        using const_iterator = const _Ty*;

        fixed_vector() noexcept : size_(0) {}

        fixed_vector(std::initializer_list<_Ty> init) : size_(0) {
            if (init.size() > _Capacity) throw std::length_error("fixed_vector: too many initializers");
            for (const auto& v : init) emplace_back(v);
        }

        ~fixed_vector() { clear(); }

        fixed_vector(const fixed_vector& other) : size_(0) {
            for (size_type i = 0; i < other.size_; ++i) {
                emplace_back(other[i]);
            }
        }

        fixed_vector& operator=(const fixed_vector& other) {
            if (this != &other) {
                clear();
                for (size_type i = 0; i < other.size_; ++i) {
                    emplace_back(other[i]);
                }
            }
            return *this;
        }

        fixed_vector(fixed_vector&& other) noexcept : size_(0) {
            for (size_type i = 0; i < other.size_; ++i) {
                emplace_back(std::move(other[i]));
            }
            other.clear();
        }

        fixed_vector& operator=(fixed_vector&& other) noexcept {
            if (this != &other) {
                clear();
                for (size_type i = 0; i < other.size_; ++i) {
                    emplace_back(std::move(other[i]));
                }
                other.clear();
            }
            return *this;
        }

        // Capacity
        constexpr size_type capacity() const noexcept { return _Capacity; }
        constexpr size_type size() const noexcept { return size_; }
        constexpr bool empty() const noexcept { return size_ == 0; }
        constexpr bool full() const noexcept { return size_ == _Capacity; }

        // Access
        reference operator[](size_type i) noexcept { return *std::launder(reinterpret_cast<_Ty*>(&data_[i])); }
        const_reference operator[](size_type i) const noexcept { return *std::launder(reinterpret_cast<const _Ty*>(&data_[i])); }

        reference at(size_type i) {
            if (i >= size_) throw std::out_of_range("fixed_vector: out of range");
            return (*this)[i];
        }
        const_reference at(size_type i) const {
            if (i >= size_) throw std::out_of_range("fixed_vector: out of range");
            return (*this)[i];
        }

        reference front() noexcept { return (*this)[0]; }
        const_reference front() const noexcept { return (*this)[0]; }
        reference back() noexcept { return (*this)[size_ - 1]; }
        const_reference back() const noexcept { return (*this)[size_ - 1]; }

        // Iterators
        iterator begin() noexcept { return &(*this)[0]; }
        const_iterator begin() const noexcept { return &(*this)[0]; }
        iterator end() noexcept { return &(*this)[size_]; }
        const_iterator end() const noexcept { return &(*this)[size_]; }

        _Ty* data() noexcept { return reinterpret_cast<_Ty*>(data_.data()); }

        template<typename... _Args>
        iterator emplace(const_iterator pos, _Args&&... args) {
            size_type index = pos - begin();
            if (size_ >= _Capacity)
                throw std::length_error("fixed_vector: capacity exceeded");
            if (index > size_)
                throw std::out_of_range("fixed_vector: insert position out of range");

            if (size_ > 0) {
                ::new (&data_[size_]) _Ty(std::move((*this)[size_ - 1]));
                for (size_type i = size_ - 1; i > index; --i) {
                    (*this)[i] = std::move((*this)[i - 1]);
                }
                (*this)[index].~_Ty();
            }

            ::new (&data_[index]) _Ty(std::forward<_Args>(args)...);
            ++size_;
            return begin() + index;
        }

        iterator insert(const_iterator pos, const _Ty& value) {
            return emplace(pos, value);
        }

        iterator insert(const_iterator pos, _Ty&& value) {
            return emplace(pos, std::move(value));
        }

        // Modifiers
        void push_back(const _Ty& value) {
            emplace_back(value);
        }
        void push_back(_Ty&& value) {
            emplace_back(std::move(value));
        }

        template <typename... _Args>
        reference emplace_back(_Args&&... args) {
            if (size_ >= _Capacity)
                throw std::length_error("fixed_vector: capacity exceeded");
            ::new (&data_[size_]) _Ty(std::forward<_Args>(args)...);
            ++size_;
            return back();
        }

        void pop_back() {
            if (size_ == 0)
                throw std::out_of_range("fixed_vector: pop_back on empty");
            --size_;
            (*this)[size_].~_Ty();
        }

        void clear() noexcept {
            while (size_ > 0) {
                pop_back();
            }
        }

        constexpr std::span<_Ty> as_span() noexcept {
            return std::span<_Ty>(begin(), size());
        }

        constexpr std::span<const _Ty> as_span() const noexcept {
            return std::span<const _Ty>(begin(), size());
        }

        constexpr operator std::span<_Ty>() noexcept { return as_span(); }
        constexpr operator std::span<const _Ty>() const noexcept { return as_span(); }

    private:
        using storage_type = std::aligned_storage_t<sizeof(_Ty), alignof(_Ty)>;
        std::array<storage_type, _Capacity> data_;
        size_type size_;
	};
}