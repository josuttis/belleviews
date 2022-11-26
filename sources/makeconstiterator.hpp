#ifndef MAKECONSTITERATOR_HPP
#define MAKECONSTITERATOR_HPP

#include <functional>
#include <ranges>
#include <span>
#include <compare>
#include <iterator>
#include <concepts>

namespace belleviews::_intern {


template <std::input_iterator Iterator>
class basic_const_iterator;


// concept NotAConstIterator:
//   Given some type I, the concept not-a-const-iterator is defined as
//   - false if I is a specialization of basic_const_iterator and
//   - true otherwise.
template<typename _Tp>
inline constexpr bool is_not_a_const_iterator = false;

template<typename _Tp>
inline constexpr bool is_not_a_const_iterator<basic_const_iterator<_Tp>> = true;

template <typename Iterator>
concept NotAConstIterator = is_not_a_const_iterator<Iterator>;


// The member typedef-name iterator_category is defined if and only if Iterator models forward_iterator.
// In that case, basic_const_iterator<Iterator>::iterator_category denotes the type
// iterator_traits<Iterator>::iterator_category.
template<typename It>
struct const_iterator_iter_cat {
};
template <std::forward_iterator It>
struct const_iterator_iter_cat<It> {
    using iterator_category = typename std::iterator_traits<It>::iterator_category;
};


template <std::input_iterator Iterator>
class basic_const_iterator : public const_iterator_iter_cat<Iterator>
{
 private:
  // basic_const_iterator<Iterator>::iterator_concept is defined as follows:
  // - If Iterator models contiguous_iterator, then iterator_concept denotes contiguous_iterator_tag.
  // - Otherwise, if Iterator models random_access_iterator, then iterator_concept denotes random_access_iterator_tag.
  // - Otherwise, if Iterator models bidirectional_iterator, then iterator_concept denotes bidirectional_iterator_tag.
  // - Otherwise, if Iterator models forward_iterator, then iterator_concept denotes forward_iterator_tag.
  // - Otherwise, iterator_concept denotes input_iterator_tag.
  static auto _S_iter_concept() {
    if constexpr (std::contiguous_iterator<Iterator>)
      return std::contiguous_iterator_tag{};
    else if constexpr (std::random_access_iterator<Iterator>)
      return std::random_access_iterator_tag{};
    else if constexpr (std::bidirectional_iterator<Iterator>)
      return std::bidirectional_iterator_tag{};
    else if constexpr (std::forward_iterator<Iterator>)
      return std::forward_iterator_tag{};
    else
      return std::input_iterator_tag{};
  }

private:
  Iterator current_ = Iterator();                                             // exposition only
  using reference = ::belleviews::_intern::iter_const_reference_t<Iterator>;  // exposition only

public:
  using iterator_concept = decltype(_S_iter_concept());
  //using iterator_category = see below ; // not always present (defined via base type)
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

  // ++ :
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

  // -- :
  constexpr basic_const_iterator& operator--() requires std::bidirectional_iterator<Iterator> {
    --current_;
    return *this;
  }
  constexpr basic_const_iterator operator--(int) requires std::bidirectional_iterator<Iterator> {
    auto tmp = *this;
    --*this;
    return tmp;
  }

  // + :
  friend constexpr basic_const_iterator operator+(const basic_const_iterator& i, difference_type n)
  requires std::random_access_iterator<Iterator> {
    return basic_const_iterator(i.current_ + n);
  }
  friend constexpr basic_const_iterator operator+(difference_type n, const basic_const_iterator& i)
  requires std::random_access_iterator<Iterator> {
    return basic_const_iterator(i.current_ + n);
  }
  
  // - :
  friend constexpr basic_const_iterator operator-(const basic_const_iterator& i, difference_type n)
  requires std::random_access_iterator<Iterator> {
    return basic_const_iterator(i.current_ - n);
  }

  // STD:
  // TODO: as friend function this yields an ERROR on gcc: 
  //template<std::sized_sentinel_for<Iterator> S>
  //friend constexpr difference_type operator-(const basic_const_iterator& x, const S& y) {
  //  return x.current_ - y;
  //}
    template <std::sized_sentinel_for<Iterator> S>
    std::iter_difference_t<Iterator> operator-(S const& s) const {
      return current_ - s;
    }

  // OOPS: it seems the order of the constraints matters:
  //  template<std::sized_sentinel_for<Iterator> S> requires belleviews::_intern::different_from<S, basic_const_iterator>  // ERROR
  //  template <typename S> requires belleviews::_intern::different_from<S, basic_const_iterator> && std::sized_sentinel_for<S, Iterator>  // OK
  template <typename S>
    //requires std::sized_sentinel_for<S, Iterator> && belleviews::_intern::different_from<S, basic_const_iterator>  // ERROR with gcc
    requires belleviews::_intern::different_from<S, basic_const_iterator> && std::sized_sentinel_for<S, Iterator>    // OK
    friend constexpr difference_type operator-(const S& x, basic_const_iterator const& y) {
        return x - y.current_;
    }

  // += and -= :
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

  // [] :
  constexpr reference operator[](difference_type n) const
    requires std::random_access_iterator<Iterator> {
    return static_cast<reference>(current_[n]);
  }

  // comparisions:
#ifdef STD
  // STD: ERROR:
  template<std::sentinel_for<Iterator> S>
  friend constexpr bool operator==(const basic_const_iterator& x, const S& s) {
    return x.current_ == s;
  }

#else
    template <std::sentinel_for<Iterator> S>
    constexpr bool operator==(S const& s) const {
        return current_ == s;
    }
#endif

  //STD
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

  template<belleviews::_intern::NotAConstIterator I>
  friend constexpr bool operator<(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x < y.current_;
  }
  template<belleviews::_intern::NotAConstIterator I>
  friend constexpr bool operator>(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x > y.current_;
  }
  template<belleviews::_intern::NotAConstIterator I>
  friend constexpr bool operator<=(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x <= y.current_;
  }
  template<belleviews::_intern::NotAConstIterator I>
  friend constexpr bool operator>=(const I& x, const basic_const_iterator& y)
  requires std::random_access_iterator<Iterator> && std::totally_ordered_with<Iterator, I> {
      return x >= y.current_;
  }
  // note: no operator<=> here
  
};


// **** According to C++23:

// template<std::input_iterator I>
// using const_iterator = see below ;
//   Result: If I models constant-iterator, I. Otherwise, basic_const_iterator<I>.

template <std::input_iterator It>
constexpr auto MakeConstIterator(It it)
{
  if constexpr (ConstantIterator<It>) {
      return it;
  }
  else {
      return basic_const_iterator<It>(it);
  }
}

template<std::input_iterator It>
using const_iterator = decltype(belleviews::_intern::MakeConstIterator(std::declval<It>()));

template<std::input_iterator I>
constexpr const_iterator<I> make_const_iterator(I it)
{
  return it;
}

// template<class S>
// using const_sentinel = see below ;
//   Result: If S models input_iterator, const_iterator<S>. Otherwise, S.

template <typename S>
constexpr auto MakeConstSentinel(S s)
{
  if constexpr (std::input_iterator<S>) {
    return const_iterator<S>(s);
  }
  else {
    return s;
  }
}

template<typename S>
using const_sentinel = decltype(belleviews::_intern::MakeConstSentinel(std::declval<S>()));

template<typename S>
constexpr const_sentinel<S> make_const_sentinel(S s)
{
  return s;
}


template<std::ranges::range R>
using const_iterator_t = const_iterator<std::ranges::iterator_t<R>>;

template<std::ranges::range R>
using const_sentinel_t = const_sentinel<std::ranges::sentinel_t<R>>;
  


}// namespace belleviews::_intern


#endif // MAKECONSTITERATOR_HPP

