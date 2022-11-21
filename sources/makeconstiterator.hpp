#ifndef MAKECONSTITERATOR_HPP
#define MAKECONSTITERATOR_HPP

#include <functional>
#include <concepts>
#include <ranges>
#include <span>
#include <compare>


// **** According to C++23:

namespace belleviews::_intern {
  template<class I>
  concept not_a_const_iterator = //see below ;
    // Given some type I, the concept not-a-const-iterator is defined as
    // false if I is a specialization of basic_const_iterator and true otherwise.
    // TODO
    false;
}

namespace std {

template<input_iterator Iterator>
class basic_const_iterator
{
 private:
  Iterator current_ = Iterator(); // exposition only
  using reference = ::belleviews::_intern::iter_const_reference_t<Iterator>; // exposition only
public:
  //using iterator_concept = see below ;
  //using iterator_category = see below ; // not always present
  using value_type = iter_value_t<Iterator>;
  using difference_type = iter_difference_t<Iterator>;

  // constructors:
  basic_const_iterator() requires default_initializable<Iterator> = default;

  constexpr basic_const_iterator(Iterator current)
   : current_{std::move(current)} {
  }

  template<std::convertible_to<Iterator> U>
    constexpr basic_const_iterator(basic_const_iterator<U> current)
   : current_{std::move(current.current_)} {
  }

  template<belleviews::_intern::different_from<basic_const_iterator> T>
    requires std::convertible_to<T, Iterator>
    constexpr basic_const_iterator(T&& current)
   : current_{std::forward<T>(current)} {
  }

  // base():
  constexpr const Iterator& base() const& noexcept { return current_; }
  constexpr Iterator base() && { return std::move(current_); }

  // * and -> :
  constexpr reference operator*() const {
    return static_cast<reference>(*current_);
  }

  constexpr const value_type* operator->() const
    requires std::is_lvalue_reference_v<iter_reference_t<Iterator>> &&
             std::same_as<std::remove_cvref_t<iter_reference_t<Iterator>>, value_type> {
    if constexpr (std::contiguous_iterator<Iterator>) {
      return to_address(current_);
    }
    else {
      std::addressof(*current_);
    }
  }

  // ++ and -- :
  constexpr basic_const_iterator& operator++() {
    ++current_;
    return *this;
  }
  constexpr void operator++(int) {
    return ++current_;
  }
  constexpr basic_const_iterator operator++(int) requires forward_iterator<Iterator> {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  constexpr basic_const_iterator& operator--() requires bidirectional_iterator<Iterator> {
    --current_;
    return *this;
  }
  constexpr basic_const_iterator operator--(int) requires bidirectional_iterator<Iterator> {
    auto tmp = *this;
    --*this;
    return tmp;
  }

  constexpr basic_const_iterator& operator+=(difference_type n)
    requires random_access_iterator<Iterator> {
      current_ += n;
      return *this;
  }
  constexpr basic_const_iterator& operator-=(difference_type n)
    requires random_access_iterator<Iterator> {
      current_ += n;
      return *this;
  }

  constexpr reference operator[](difference_type n) const
    requires random_access_iterator<Iterator> {
    return static_cast<reference>(current_[n]);
  }

  // comparisions:
  template<sentinel_for<Iterator> S>
  friend constexpr bool operator==(const basic_const_iterator& x, const S& s) {
    return x.current_ == s;
  }

  friend constexpr bool operator<(const basic_const_iterator& x, const basic_const_iterator& y)
    requires random_access_iterator<Iterator> {
      return x.current_ < y.current_;
  }
  friend constexpr bool operator>(const basic_const_iterator& x, const basic_const_iterator& y)
    requires random_access_iterator<Iterator> {
      return x.current_ > y.current_;
  }
  friend constexpr bool operator<=(const basic_const_iterator& x, const basic_const_iterator& y)
    requires random_access_iterator<Iterator> {
      return x.current_ <= y.current_;
  }
  friend constexpr bool operator>=(const basic_const_iterator& x, const basic_const_iterator& y)
    requires random_access_iterator<Iterator> {
      return x.current_ >= y.current_;
  }
  friend constexpr auto operator<=>(const basic_const_iterator& x, const basic_const_iterator& y)
    requires random_access_iterator<Iterator> && three_way_comparable<Iterator> {
      return x.current_ <=> y.current_;
  }

  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr bool operator<(const basic_const_iterator& x, const I& y)
    requires random_access_iterator<Iterator> && totally_ordered_with<Iterator, I> {
      return x.current_ < y;
  }
  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr bool operator>(const basic_const_iterator& x, const I& y)
    requires random_access_iterator<Iterator> && totally_ordered_with<Iterator, I> {
      return x.current_ > y.current_;
  }
  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr bool operator<=(const basic_const_iterator& x, const I& y)
  requires random_access_iterator<Iterator> && totally_ordered_with<Iterator, I> {
      return x.current_ <= y;
  }
  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr bool operator>=(const basic_const_iterator& x, const I& y)
  requires random_access_iterator<Iterator> && totally_ordered_with<Iterator, I> {
      return x.current_ >= y;
  }
  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr auto operator<=>(const basic_const_iterator& x, const I& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> &&
           std::three_way_comparable_with<Iterator, I> {
      return x.current_ <=> y;
  }
  
  template<belleviews::_intern::not_a_const_iterator I>
  friend constexpr bool operator<(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x < y.current_;
  }
  template<belleviews::_intern::not_a_const_iterator I>
  friend constexpr bool operator>(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x > y.current_;
  }
  template<belleviews::_intern::not_a_const_iterator I>
  friend constexpr bool operator<=(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x <= y.current_;
  }
  template<belleviews::_intern::not_a_const_iterator I>
  friend constexpr bool operator>=(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x >= y.current_;
  }
  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr auto operator<=>(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> &&
           std::three_way_comparable_with<Iterator, I> {
      return x <=> y.current_;
  }
  
  // + and - :
  friend constexpr basic_const_iterator operator+(const basic_const_iterator& i, difference_type n)
  requires random_access_iterator<Iterator> {
    return basic_const_iterator(i.current_ + n);
  }
  
  friend constexpr basic_const_iterator operator+(difference_type n, const basic_const_iterator& i)
  requires random_access_iterator<Iterator> {
    return basic_const_iterator(i.current_ + n);
  }
  
  friend constexpr basic_const_iterator operator-(const basic_const_iterator& i, difference_type n)
  requires random_access_iterator<Iterator> {
    return basic_const_iterator(i.current_ - n);
  }
  
  template<std::sized_sentinel_for<Iterator> S>
  friend constexpr difference_type operator-(const basic_const_iterator& i, const S& y) {
    return basic_const_iterator(i.current_ - y);
  }
  
  template<std::sized_sentinel_for<Iterator> S>
  requires belleviews::_intern::different_from<S, basic_const_iterator>
  friend constexpr difference_type operator-(const S& x, const basic_const_iterator& y) {
    return x - y.current_;
  }
};

}// namespace std


// **** According to C++23:
//
// template<input_iterator I>
// using const_iterator = see below ; // freestanding
//   Result: If I models constant-iterator, I. Otherwise, basic_const_iterator<I>.
//
// template<class S>
// using const_sentinel = //see below ; // freestanding
//   Result: If S models input_iterator, const_iterator<S>. Otherwise, S.

namespace belleviews::_intern {
  template<typename It>
  concept constant_iterator = // exposition only
    std::input_iterator<It> &&
    std::same_as<iter_const_reference_t<It>, std::iter_reference_t<It>>;
}

namespace std {

// make_const_iterator() and const_iterator:
template <typename It>
constexpr auto make_const_iterator(It i)
{
  if constexpr (belleviews::_intern::constant_iterator<It>) {
    return i;
  }
  else {
    return basic_const_iterator<It>(i);
  }
}

template <std::input_iterator It>
using const_iterator = decltype(make_const_iterator(std::declval<It>()));


// make_const_sentinel() and const_sentinel:
template <typename S>
constexpr auto make_const_sentinel(S s)
{
  if constexpr (std::input_iterator<S>) {
    return make_const_iterator(s);
  }
  else {
    return s;
  }
}

template <typename S>
using const_sentinel = decltype(make_const_sentinel(std::declval<S>()));

}// namespace std

#endif // MAKECONSTITERATOR_HPP

