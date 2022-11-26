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
  template<typename _Tp, typename _Up>
  concept different_from = !std::same_as<std::remove_cvref_t<_Tp>, std::remove_cvref_t<_Up>>;


  // is_initializer_list
  template<typename _Tp>
  inline constexpr bool is_initializer_list = false;

  template<typename _Tp>
  inline constexpr bool is_initializer_list<std::initializer_list<_Tp>> = true;

  // maybe_const
  template<bool _Const, typename _Tp>
    using maybe_const_t = std::conditional_t<_Const, const _Tp, _Tp>;

  // can_reference:
  template<typename _Tp>
    using with_ref = _Tp&;

  template<typename _Tp>
    concept can_reference = requires { typename with_ref<_Tp>; };


  // new range types of C++23:
  template<std::ranges::range R>
  using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;

  template<std::indirectly_readable It>
  using iter_const_rvalue_reference_t = std::common_reference_t<const std::iter_value_t<It>&&, std::iter_rvalue_reference_t<It>>;
  template<std::ranges::range R>
  using range_const_rvalue_reference_t = iter_const_rvalue_reference_t<std::ranges::iterator_t<R>>;


  // box type (semiregular box)
  template<typename V>
  concept boxable = std::copy_constructible<V> && std::is_object_v<V>;

  template<boxable V>
  struct box : std::optional<V>
  {
    using std::optional<V>::optional;

    constexpr box() noexcept(std::is_nothrow_default_constructible_v<V>) requires std::default_initializable<V>
     : std::optional<V>{std::in_place} {
    }

    box(const box&) = default;
    box(box&&) = default;

    using std::optional<V>::operator=;

    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 3477. Simplify constraints for semiregular-box
    // 3572. copyable-box should be fully constexpr
    constexpr box& operator=(const box& rhs) noexcept(std::is_nothrow_copy_constructible_v<V>)
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

    constexpr box& operator=(box&& rhs) noexcept(std::is_nothrow_move_constructible_v<V>)
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
  // TODO: optimize box as in gcc (__box there)

  // convertible_to_non_slicing
  template<typename _From, typename _To>
      concept uses_nonqualification_pointer_conversion
	= std::is_pointer_v<_From> && std::is_pointer_v<_To>
	  && !std::convertible_to<std::remove_pointer_t<_From>(*)[],
			          std::remove_pointer_t<_To>(*)[]>;

  template<typename _From, typename _To>
      concept convertible_to_non_slicing = std::convertible_to<_From, _To>
	&& !uses_nonqualification_pointer_conversion<std::decay_t<_From>,
						     std::decay_t<_To>>;

  // pair_like_convertible_from
  template<typename _Tp>
      concept pair_like
	= !std::is_reference_v<_Tp> && requires(_Tp __t)
	{
	  typename std::tuple_size<_Tp>::type;
	  requires std::derived_from<std::tuple_size<_Tp>, std::integral_constant<size_t, 2>>;
	  typename std::tuple_element_t<0, std::remove_const_t<_Tp>>;
	  typename std::tuple_element_t<1, std::remove_const_t<_Tp>>;
	  { get<0>(__t) } -> std::convertible_to<const std::tuple_element_t<0, _Tp>&>;
	  { get<1>(__t) } -> std::convertible_to<const std::tuple_element_t<1, _Tp>&>;
	};

  template<typename _Tp, typename _Up, typename _Vp>
      concept pair_like_convertible_from
	= !std::ranges::range<_Tp> && pair_like<_Tp>
	&& std::constructible_from<_Tp, _Up, _Vp>
	&& convertible_to_non_slicing<_Up, std::tuple_element_t<0, _Tp>>
	&& std::convertible_to<_Vp, std::tuple_element_t<1, _Tp>>;

  // make_unsigned_like:
  using max_size_type = std::size_t;   // TODO: maybe to use gcc definition instead
  using max_diff_type = long long;     // TODO: maybe to use gcc definition instead
  constexpr max_size_type
    to_unsigned_like(max_size_type __t) noexcept
    { return __t; }

  constexpr max_size_type
    to_unsigned_like(max_diff_type __t) noexcept
    { return max_size_type(__t); }

  template<std::integral _Tp>
      constexpr auto
      to_unsigned_like(_Tp __t) noexcept
      { return static_cast<std::make_unsigned_t<_Tp>>(__t); }

#if defined __STRICT_ANSI__ && defined __SIZEOF_INT128__
  constexpr unsigned __int128
    to_unsigned_like(__int128 __t) noexcept
    { return __t; }

  constexpr unsigned __int128
    to_unsigned_like(unsigned __int128 __t) noexcept
    { return __t; }
#endif

  template<typename _Tp>
      using make_unsigned_like_t
	= decltype(_intern::to_unsigned_like(std::declval<_Tp>()));

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
