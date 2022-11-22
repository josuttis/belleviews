#ifndef MAKECONSTITERATOR_HPP
#define MAKECONSTITERATOR_HPP

#include <functional>
#include <concepts>
#include <ranges>
#include <span>
#include <compare>


////////////////////////////////////////////////////////////////////////
// https://brevzin.github.io/c++/2019/09/23/declarative-cpos/

template <typename It> struct iterator_concept_for { };
template <typename It> requires std::contiguous_iterator<It>
struct iterator_concept_for<It> {
    using iterator_concept = std::contiguous_iterator_tag;
};

template <typename It> struct iterator_category_for { };
template <std::forward_iterator It>
struct iterator_category_for<It> {
    using iterator_category = typename std::iterator_traits<It>::iterator_category;
};

template <std::input_iterator Iterator>
using const_ref_for = std::common_reference_t<std::iter_value_t<Iterator> const &&, std::iter_reference_t<Iterator>>;

template <typename Iterator>
concept ConstantIterator = std::input_iterator<Iterator>
                        && std::same_as<const_ref_for<Iterator>, std::iter_reference_t<Iterator>>;

template <typename R>
concept ConstantRange = std::ranges::range<R> && ConstantIterator<std::ranges::iterator_t<R>>;

template <typename T, typename U>
concept NotSameAs = not std::same_as<T, U>;

template <std::input_iterator Iterator>
class basic_const_iterator : public iterator_concept_for<Iterator>
                           , public iterator_category_for<Iterator>
{
    Iterator current_;

public:
    using value_type = std::iter_value_t<Iterator>;
    using difference_type = std::iter_difference_t<Iterator>;
    using reference = const_ref_for<Iterator>;

    basic_const_iterator() = default;
    basic_const_iterator(Iterator it) : current_(std::move(it)) { }
    template <std::convertible_to<Iterator> U>
    basic_const_iterator(basic_const_iterator<U> c) : current_(std::move(c.base())) { }
    basic_const_iterator(std::convertible_to<Iterator> auto&& c) : current_(FWD(c)) { }

    auto operator++() -> basic_const_iterator& { ++current_; return *this; }
    auto operator++(int) -> basic_const_iterator requires std::forward_iterator<Iterator> { auto cpy = *this; ++*this; return cpy; }        
    void operator++(int) { ++*this; }

    auto operator--() -> basic_const_iterator& requires std::bidirectional_iterator<Iterator> { --current_; return *this; }
    auto operator--(int) -> basic_const_iterator requires std::bidirectional_iterator<Iterator> { auto cpy = *this; --*this; return cpy; }        

    auto operator+(difference_type n) const -> basic_const_iterator requires std::random_access_iterator<Iterator> { return const_iterator(current_ + n); }
    auto operator-(difference_type n) const -> basic_const_iterator requires std::random_access_iterator<Iterator> { return const_iterator(current_ - n); }
    friend auto operator+(difference_type n, basic_const_iterator const& rhs) -> basic_const_iterator { return rhs + n; }
    auto operator+=(difference_type n) -> basic_const_iterator& requires std::random_access_iterator<Iterator> { current_ += n; return *this; }
    auto operator-=(difference_type n) -> basic_const_iterator& requires std::random_access_iterator<Iterator> { current_ -= n; return *this; }        
    auto operator-(basic_const_iterator const& rhs) const -> difference_type requires std::random_access_iterator<Iterator> { return current_ - rhs.current_; }
    auto operator[](difference_type n) const -> reference requires std::random_access_iterator<Iterator> { return current_[n]; }

    auto operator*() const -> reference { return *current_; }
    auto operator->() const -> value_type const* requires std::contiguous_iterator<Iterator> { return std::to_address(current_); }

    template <std::sentinel_for<Iterator> S>
    auto operator==(S const& s) const -> bool {
        return current_ == s;
    }

    auto operator<=>(basic_const_iterator const& rhs) const requires std::random_access_iterator<Iterator> {
        return current_ <=> rhs;
    }

    template <NotSameAs<basic_const_iterator> Rhs>
        requires std::random_access_iterator<Iterator>
             and std::totally_ordered_with<Iterator, Rhs>
    auto operator<=>(Rhs const& rhs) const {
        if constexpr (std::three_way_comparable_with<Iterator, Rhs>) {
            return current_ <=> rhs;
        } else if constexpr (std::sized_sentinel_for<Rhs, Iterator>) {
            return (current_ - rhs) <=> 0;
        } else {
            if (current_ < rhs) return std::strong_ordering::less;
            if (rhs < current_) return std::strong_ordering::greater;
            return std::strong_ordering::equal;
        }
    }

    template <std::sized_sentinel_for<Iterator> S>
    auto operator-(S const& s) const -> std::iter_difference_t<Iterator> {
        return current_ - s;
    }

    template <NotSameAs<basic_const_iterator> S>
        requires std::sized_sentinel_for<S, Iterator>
    friend auto operator-(S const& s, basic_const_iterator const& rhs) -> std::iter_difference_t<Iterator> {
        return s - rhs.current_;
    }

    auto base() -> Iterator& { return current_; }
    auto base() const -> Iterator const& { return current_; }
};


// **** According to C++23:

/*
namespace belleviews::_intern {
  template<class I>
  concept not_a_const_iterator = //see below ;
    // Given some type I, the concept not-a-const-iterator is defined as
    // false if I is a specialization of basic_const_iterator and true otherwise.
    // TODO
    false;
}

//namespace std {

template<std::input_iterator Iterator>
class basic_const_iterator
{
 private:
  Iterator current_ = Iterator(); // exposition only
  using reference = ::belleviews::_intern::iter_const_reference_t<Iterator>; // exposition only
public:
  //using iterator_concept = see below ;
  //using iterator_category = see below ; // not always present
  using value_type = std::iter_value_t<Iterator>;
  using difference_type = std::iter_difference_t<Iterator>;

  // constructors:
  basic_const_iterator() requires std::default_initializable<Iterator> = default;

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
    requires std::is_lvalue_reference_v<std::iter_reference_t<Iterator>> &&
             std::same_as<std::remove_cvref_t<std::iter_reference_t<Iterator>>, value_type> {
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
  constexpr basic_const_iterator operator++(int) requires std::forward_iterator<Iterator> {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  constexpr basic_const_iterator& operator--() requires std::bidirectional_iterator<Iterator> {
    --current_;
    return *this;
  }
  constexpr basic_const_iterator operator--(int) requires std::bidirectional_iterator<Iterator> {
    auto tmp = *this;
    --*this;
    return tmp;
  }

  constexpr basic_const_iterator& operator+=(difference_type n)
    requires std::random_access_iterator<Iterator> {
      current_ += n;
      return *this;
  }
  constexpr basic_const_iterator& operator-=(difference_type n)
    requires std::random_access_iterator<Iterator> {
      current_ += n;
      return *this;
  }

  constexpr reference operator[](difference_type n) const
    requires std::random_access_iterator<Iterator> {
    return static_cast<reference>(current_[n]);
  }

  // comparisions:
  template<std::sentinel_for<Iterator> S>
  friend constexpr bool operator==(const basic_const_iterator& x, const S& s) {
    return x.current_ == s;
  }

  friend constexpr bool operator<(const basic_const_iterator& x, const basic_const_iterator& y)
    requires std::random_access_iterator<Iterator> {
      return x.current_ < y.current_;
  }
  friend constexpr bool operator>(const basic_const_iterator& x, const basic_const_iterator& y)
    requires std::random_access_iterator<Iterator> {
      return x.current_ > y.current_;
  }
  friend constexpr bool operator<=(const basic_const_iterator& x, const basic_const_iterator& y)
    requires std::random_access_iterator<Iterator> {
      return x.current_ <= y.current_;
  }
  friend constexpr bool operator>=(const basic_const_iterator& x, const basic_const_iterator& y)
    requires std::random_access_iterator<Iterator> {
      return x.current_ >= y.current_;
  }
  friend constexpr auto operator<=>(const basic_const_iterator& x, const basic_const_iterator& y)
    requires std::random_access_iterator<Iterator> && std::three_way_comparable<Iterator> {
      return x.current_ <=> y.current_;
  }

  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr bool operator<(const basic_const_iterator& x, const I& y)
    requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x.current_ < y;
  }
  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr bool operator>(const basic_const_iterator& x, const I& y)
    requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x.current_ > y.current_;
  }
  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr bool operator<=(const basic_const_iterator& x, const I& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x.current_ <= y;
  }
  template<belleviews::_intern::different_from<basic_const_iterator> I>
  friend constexpr bool operator>=(const basic_const_iterator& x, const I& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
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
  requires std::random_access_iterator<Iterator> {
    return basic_const_iterator(i.current_ + n);
  }
  
  friend constexpr basic_const_iterator operator+(difference_type n, const basic_const_iterator& i)
  requires std::random_access_iterator<Iterator> {
    return basic_const_iterator(i.current_ + n);
  }
  
  friend constexpr basic_const_iterator operator-(const basic_const_iterator& i, difference_type n)
  requires std::random_access_iterator<Iterator> {
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

//}// namespace std
  */


template <typename T, std::common_with<T> U>
struct std::common_type<basic_const_iterator<T>, U> {
    using type = basic_const_iterator<std::common_type_t<T, U>>;
};
template <typename T, std::common_with<T> U>
struct std::common_type<U, basic_const_iterator<T>> {
    using type = basic_const_iterator<std::common_type_t<T, U>>;
};
template <typename T, std::common_with<T> U>
struct std::common_type<basic_const_iterator<T>, basic_const_iterator<U>> {
    using type = basic_const_iterator<std::common_type_t<T, U>>;
};

/*
// According to C++23:
template<input_iterator I>
constexpr const_iterator<I> make_const_iterator(I it)
{
  return it;
}

template<class S>
constexpr const_sentinel<S> make_const_sentinel(S s)
{
  return s;
}
*/

template <std::input_iterator It>
constexpr auto make_const_iterator(It it) -> ConstantIterator auto
{
    if constexpr (ConstantIterator<It>) {
        return it;
    } else {
        return basic_const_iterator<It>(it);
    }
}

template <std::input_iterator It>
using const_iterator = decltype(make_const_iterator(std::declval<It>()));

template <typename S>
constexpr auto make_const_sentinel(S s)
{
  if constexpr (std::input_iterator<S>) {
    return make_const_iterator(std::move(s));
  } else {
    return s;
  }
}

//////////////////////////




// **** According to C++23:
//
// template<input_iterator I>
// using const_iterator = see below ; // freestanding
//   Result: If I models constant-iterator, I. Otherwise, basic_const_iterator<I>.
//
// template<class S>
// using const_sentinel = //see below ; // freestanding
//   Result: If S models input_iterator, const_iterator<S>. Otherwise, S.

/*
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
*/

#endif // MAKECONSTITERATOR_HPP

