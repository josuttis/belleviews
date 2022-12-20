// <belleviewsutils.hpp> -*- C++ -*-
//
// Copyright (C) 2019-2022 Free Software Foundation, Inc.
// Copyright (C) 2022 Nicolai Josuttis
//
// This file is part of the belleviews library,
// which is using parts of the GNU ISO C++ Library.  
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3,
// or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.
//
// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef BELLEVIEWSUTILS_HPP
#define BELLEVIEWSUTILS_HPP

#include <optional>

//**********************************************************************
// General utilities
// a) either not in C++20 yet
// b) or exposition only concepts etc.
// NOTE: the const_iterator stuff is in "makeconstiterator.hpp"
//**********************************************************************

namespace belleviews::_intern {

  // new alias templates in C++23 [const.iterators.alias]:
  template<std::indirectly_readable It>
  using iter_const_reference_t = std::common_reference_t<const std::iter_value_t<It>&&, std::iter_reference_t<It>>;

  template <typename It>
  concept ConstantIterator = std::input_iterator<It> &&
                             std::same_as<iter_const_reference_t<It>, std::iter_reference_t<It>>;

  // in makeconstiterator.hpp:
  //template<input_iterator I>
  //using const_iterator = see below ;
  //template<class S>
  //using const_sentinel = see below ;


  // internal concepts in [range.utility.helpers]

  // concept simple-view:
  template<typename R>
  concept simple_view = std::ranges::view<R> && std::ranges::range<const R>
                         && std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>>
                         && std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

  // concept has-arrow:
  template<typename It>
  concept has_arrow = std::input_iterator<It> &&
                      (std::is_pointer_v<It> || requires(It it) { it.operator->(); });

  // concept different-from:
  template<typename T, typename U>
  concept different_from = !std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;


  // is_initializer_list
  template<typename T>
  inline constexpr bool is_initializer_list = false;

  template<typename T>
  inline constexpr bool is_initializer_list<std::initializer_list<T>> = true;

  // maybe_const
  template<bool ConstT, typename T>
    using maybe_const_t = std::conditional_t<ConstT, const T, T>;

  // can_reference:
  template<typename T>
    using with_ref = T&;

  template<typename T>
    concept can_reference = requires { typename with_ref<T>; };


  // new range types of C++23:
  template<std::ranges::range R>
  using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;

  template<std::indirectly_readable It>
  using iter_const_rvalue_reference_t = std::common_reference_t<const std::iter_value_t<It>&&, std::iter_rvalue_reference_t<It>>;
  template<std::ranges::range R>
  using range_const_rvalue_reference_t = iter_const_rvalue_reference_t<std::ranges::iterator_t<R>>;


  // semiregular-box type
  template<typename V>
  concept boxable = std::copy_constructible<V> && std::is_object_v<V>;

  template<boxable V>
  struct SemiregBox : std::optional<V>
  {
    using std::optional<V>::optional;

    constexpr SemiregBox() noexcept(std::is_nothrow_default_constructible_v<V>) requires std::default_initializable<V>
     : std::optional<V>{std::in_place} {
    }

    SemiregBox(const SemiregBox&) = default;
    SemiregBox(SemiregBox&&) = default;

    using std::optional<V>::operator=;

    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 3477. Simplify constraints for semiregular-box
    // 3572. copyable-box should be fully constexpr
    constexpr SemiregBox& operator=(const SemiregBox& rhs) noexcept(std::is_nothrow_copy_constructible_v<V>)
    requires (!std::copyable<V>) {
      if (this != std::addressof(rhs)) {
        if ((bool)rhs) {
          this->emplace(*rhs);
        }
        else {
          this->reset();
        }
      }
      return *this;
    }

    constexpr SemiregBox& operator=(SemiregBox&& rhs) noexcept(std::is_nothrow_move_constructible_v<V>)
    requires (!std::movable<V>) {
      if (this != std::addressof(rhs)) {
        if ((bool)rhs) {
          this->emplace(std::move(*rhs));
        }
        else {
          this->reset();
        }
      }
      return *this;
    }
  };
  // TODO: optimize SemiregBox as in gcc (__box there)

  // convertible_to_non_slicing
  template<typename From, typename To>
      concept uses_nonqualification_pointer_conversion
	= std::is_pointer_v<From> && std::is_pointer_v<To>
	  && !std::convertible_to<std::remove_pointer_t<From>(*)[],
			          std::remove_pointer_t<To>(*)[]>;

  template<typename From, typename To>
      concept convertible_to_non_slicing = std::convertible_to<From, To>
	&& !uses_nonqualification_pointer_conversion<std::decay_t<From>,
						     std::decay_t<To>>;

  // pair_like_convertible_from
  template<typename T>
      concept pair_like
	= !std::is_reference_v<T> && requires(T __t)
	{
	  typename std::tuple_size<T>::type;
	  requires std::derived_from<std::tuple_size<T>, std::integral_constant<size_t, 2>>;
	  typename std::tuple_element_t<0, std::remove_const_t<T>>;
	  typename std::tuple_element_t<1, std::remove_const_t<T>>;
	  { get<0>(__t) } -> std::convertible_to<const std::tuple_element_t<0, T>&>;
	  { get<1>(__t) } -> std::convertible_to<const std::tuple_element_t<1, T>&>;
	};

  template<typename T, typename U, typename V>
      concept pair_like_convertible_from
	= !std::ranges::range<T> && pair_like<T>
	&& std::constructible_from<T, U, V>
	&& convertible_to_non_slicing<U, std::tuple_element_t<0, T>>
	&& std::convertible_to<V, std::tuple_element_t<1, T>>;

  // make_unsigned_like:
  using max_size_type = std::size_t;   // TODO: maybe to use gcc definition instead
  using max_diff_type = long long;     // TODO: maybe to use gcc definition instead
  constexpr max_size_type
    to_unsigned_like(max_size_type __t) noexcept
    { return __t; }

  constexpr max_size_type
    to_unsigned_like(max_diff_type __t) noexcept
    { return max_size_type(__t); }

  template<std::integral T>
      constexpr auto
      to_unsigned_like(T __t) noexcept
      { return static_cast<std::make_unsigned_t<T>>(__t); }

#if defined __STRICT_ANSI__ && defined __SIZEOF_INT128__
  constexpr unsigned __int128
    to_unsigned_like(__int128 __t) noexcept
    { return __t; }

  constexpr unsigned __int128
    to_unsigned_like(unsigned __int128 __t) noexcept
    { return __t; }
#endif

  template<typename T>
      using make_unsigned_like_t
	= decltype(_intern::to_unsigned_like(std::declval<T>()));

} // namespace belleviews::_intern



//**********************************************************************
// make_const_iterator() if not available yet
//**********************************************************************
#ifndef __cpp_lib_ranges_as_const

#include "makeconstiterator.hpp"

namespace std {
template<std::input_iterator I>
constexpr auto make_const_iterator(I it)
{
  return belleviews::_intern::make_const_iterator(it);
}

template<typename S>
constexpr auto make_const_sentinel(S s)
{
  return belleviews::_intern::make_const_sentinel(s);
}
}// namespace std

#endif // __cpp_lib_ranges_as_const

#endif // BELLEVIEWSUTILS_HPP
