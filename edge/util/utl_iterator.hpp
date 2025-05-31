#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

namespace edge::utl {
    template<typename _Iter, typename _Func>
    class transform_iterator {
    public:
        using iterator_category = typename std::iterator_traits<_Iter>::iterator_category;
        using difference_type = typename std::iterator_traits<_Iter>::difference_type;
        using reference = decltype(std::declval<_Func>()(*std::declval<_Iter>()));
        using value_type = std::remove_cvref_t<reference>;
        using pointer = value_type*;

        transform_iterator() = default;
        transform_iterator(_Iter it, _Func func) : it_(it), func_(func) {}

        reference operator*() const { return func_(*it_); }
        transform_iterator& operator++() { ++it_; return *this; }
        transform_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }

        bool operator==(const transform_iterator& other) const { return it_ == other.it_; }
        bool operator!=(const transform_iterator& other) const { return it_ != other.it_; }

        transform_iterator& operator--() { --it_; return *this; }
        transform_iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

        transform_iterator operator+(difference_type n) const { return transform_iterator(it_ + n, func_); }
        transform_iterator operator-(difference_type n) const { return transform_iterator(it_ - n, func_); }
        difference_type operator-(const transform_iterator& other) const { return it_ - other.it_; }
        bool operator<(const transform_iterator& other) const { return it_ < other.it_; }
        bool operator>(const transform_iterator& other) const { return it_ > other.it_; }
        bool operator<=(const transform_iterator& other) const { return it_ <= other.it_; }
        bool operator>=(const transform_iterator& other) const { return it_ >= other.it_; }
        reference operator[](difference_type n) const { return func_(it_[n]); }

    private:
        _Iter it_;
        _Func func_;
    };

    template<typename Iter, typename Func>
    auto make_transform_iterator(Iter it, Func func) {
        return transform_iterator<Iter, Func>(it, func);
    }
}