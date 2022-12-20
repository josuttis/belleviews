// <bellesub.hpp> -*- C++ -*-
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

#ifndef BELLESUB_HPP
#define BELLESUB_HPP

#include "makeconstiterator.hpp"

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::sub_view
// 
// A C++ view
// with the following benefits compared to C++ standard views
// - Always propagates const
// Because
// - This view yields const iterators when it is const
// OPEN/TODO:
// - propgates const
// - borrowed?
//*************************************************************
namespace belleviews {

enum class subrange_kind : bool { unsized, sized };

template<std::input_or_output_iterator It, std::sentinel_for<It> Sent = It,
         subrange_kind Kind = std::sized_sentinel_for<Sent, It> ? subrange_kind::sized
                                                              : subrange_kind::unsized>
requires (Kind == subrange_kind::sized || !std::sized_sentinel_for<Sent, It>)
class sub_view : public std::ranges::view_interface<sub_view<It, Sent, Kind>>
{
    private:
      static constexpr bool _s_store_size
        = Kind == subrange_kind::sized && !std::sized_sentinel_for<Sent, It>;

      //friend struct views::Drop; // Needs to inspect _s_store_size.

      It _m_begin = It();
      [[no_unique_address]] Sent _m_end = Sent();

      using __size_type
        = _intern::make_unsigned_like_t<std::iter_difference_t<It>>;

      template<typename, bool = _s_store_size>
        struct Size
        { };

      template<typename T>
        struct Size<T, true>
        { T _m_size; };

      [[no_unique_address]] Size<__size_type> _m_size = {};

    public:
      sub_view() requires std::default_initializable<It> = default;

      constexpr
      sub_view(_intern::convertible_to_non_slicing<It> auto __i, Sent __s)
      noexcept(std::is_nothrow_constructible_v<It, decltype(__i)>
               && std::is_nothrow_constructible_v<Sent, Sent&>)
        requires (!_s_store_size)
      : _m_begin(std::move(__i)), _m_end(__s)
      { }

      constexpr
      sub_view(_intern::convertible_to_non_slicing<It> auto __i, Sent __s,
               __size_type __n)
      noexcept(std::is_nothrow_constructible_v<It, decltype(__i)>
               && std::is_nothrow_constructible_v<Sent, Sent&>)
        requires (Kind == subrange_kind::sized)
      : _m_begin(std::move(__i)), _m_end(__s)
      {
        if constexpr (_s_store_size)
          _m_size._m_size = __n;
      }

      template<_intern::different_from<sub_view> Rng>
        requires std::ranges::borrowed_range<Rng>
          && _intern::convertible_to_non_slicing<std::ranges::iterator_t<Rng>, It>
          && std::convertible_to<std::ranges::sentinel_t<Rng>, Sent>
        constexpr
        sub_view(Rng&& __r)
        noexcept(noexcept(sub_view(__r, std::ranges::size(__r))))
        requires _s_store_size && std::ranges::sized_range<Rng>
        : sub_view(__r, std::ranges::size(__r))
        { }

      template<_intern::different_from<sub_view> Rng>
        requires std::ranges::borrowed_range<Rng>
          && _intern::convertible_to_non_slicing<std::ranges::iterator_t<Rng>, It>
          && std::convertible_to<std::ranges::sentinel_t<Rng>, Sent>
        constexpr
        sub_view(Rng&& __r)
        noexcept(noexcept(sub_view(std::ranges::begin(__r), std::ranges::end(__r))))
        requires (!_s_store_size)
        : sub_view(std::ranges::begin(__r), std::ranges::end(__r))
        { }

      template<std::ranges::borrowed_range Rng>
        requires _intern::convertible_to_non_slicing<std::ranges::iterator_t<Rng>, It>
          && std::convertible_to<std::ranges::sentinel_t<Rng>, Sent>
        constexpr
        sub_view(Rng&& __r, __size_type __n)
        noexcept(noexcept(sub_view(std::ranges::begin(__r), std::ranges::end(__r), __n)))
        requires (Kind == subrange_kind::sized)
        : sub_view{std::ranges::begin(__r), std::ranges::end(__r), __n}
        { }

      template<_intern::different_from<sub_view> PairLike>
        requires _intern::pair_like_convertible_from<PairLike, const It&,
                                                        const Sent&>
        constexpr
        operator PairLike() const
        { return PairLike(_m_begin, _m_end); }

      // begin():
      constexpr It begin() requires std::copyable<It> {
        return _m_begin;
      }
      constexpr auto begin() const requires std::copyable<It> {
        return std::make_const_iterator(_m_begin);
      }

      [[nodiscard]] constexpr It begin() requires (!std::copyable<It>) {
        return std::move(_m_begin); 
      }

      // end():
      constexpr Sent end() {
        return _m_end;
      }
      constexpr auto end() const {
        return std::make_const_sentinel(_m_end); 
      }

      constexpr bool empty() const { return _m_begin == _m_end; }

      constexpr __size_type
      size() const requires (Kind == subrange_kind::sized)
      {
        if constexpr (_s_store_size)
          return _m_size._m_size;
        else
          return _intern::to_unsigned_like(_m_end - _m_begin);
      }

      [[nodiscard]] constexpr sub_view
      next(std::iter_difference_t<It> __n = 1) const &
        requires std::forward_iterator<It>
      {
        auto __tmp = *this;
        __tmp.advance(__n);
        return __tmp;
      }

      [[nodiscard]] constexpr sub_view
      next(std::iter_difference_t<It> __n = 1) &&
      {
        advance(__n);
        return std::move(*this);
      }

      [[nodiscard]] constexpr sub_view
      prev(std::iter_difference_t<It> __n = 1) const
        requires std::bidirectional_iterator<It>
      {
        auto __tmp = *this;
        __tmp.advance(-__n);
        return __tmp;
      }

      constexpr sub_view&
      advance(std::iter_difference_t<It> __n)
      {
        // _GLIBCXX_RESOLVE_LIB_DEFECTS
        // 3433. sub_view::advance(n) has UB when n < 0
        if constexpr (std::bidirectional_iterator<It>)
          if (__n < 0)
            {
              std::ranges::advance(_m_begin, __n);
              if constexpr (_s_store_size)
                _m_size._m_size += _intern::to_unsigned_like(-__n);
              return *this;
            }

        __glibcxx_assert(__n >= 0);
        auto __d = __n - std::ranges::advance(_m_begin, __n, _m_end);
        if constexpr (_s_store_size)
          _m_size._m_size -= _intern::to_unsigned_like(__d);
        return *this;
      }
};

template<std::input_or_output_iterator It, std::sentinel_for<It> Sent>
  sub_view(It, Sent) -> sub_view<It, Sent>;

template<std::input_or_output_iterator It, std::sentinel_for<It> Sent>
    sub_view(It, Sent,
             _intern::make_unsigned_like_t<std::iter_difference_t<It>>)
  -> sub_view<It, Sent, subrange_kind::sized>;

template<std::ranges::borrowed_range Rng>
    sub_view(Rng&&)
  -> sub_view<std::ranges::iterator_t<Rng>, std::ranges::sentinel_t<Rng>,
              (std::ranges::sized_range<Rng>
               || std::sized_sentinel_for<std::ranges::sentinel_t<Rng>, std::ranges::iterator_t<Rng>>)
              ? subrange_kind::sized : subrange_kind::unsized>;

template<std::ranges::borrowed_range Rng>
    sub_view(Rng&&,
             _intern::make_unsigned_like_t<std::ranges::range_difference_t<Rng>>)
  -> sub_view<std::ranges::iterator_t<Rng>, std::ranges::sentinel_t<Rng>, subrange_kind::sized>;

template<size_t Num, class It, class Sent, subrange_kind Kind>
    requires (Num < 2)
    constexpr auto
get(const sub_view<It, Sent, Kind>& __r)
    {
      if constexpr (Num == 0)
        return __r.begin();
      else
        return __r.end();
    }

template<size_t Num, class It, class Sent, subrange_kind Kind>
    requires (Num < 2)
    constexpr auto
get(sub_view<It, Sent, Kind>&& __r)
    {
      if constexpr (Num == 0)
        return __r.begin();
      else
        return __r.end();
    }




} // namespace ranges


// ALWAYS BORROWED VIEW:
template<typename It, typename Sent, belleviews::subrange_kind Kind>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::sub_view<It, Sent, Kind>> = true;

//TODO:
//template<range Range>
//  using borrowed_subrange_t = __conditional_t<borrowed_range<Range>,
//                                                sub_view<std::ranges::iterator_t<Range>>,
//                                                dangling>;


//*************************************************************
// belleviews::sub()
// bel::views::sub()
// 
// A C++ sub_view factory for the belleviews::sub_view
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename It, typename Sent>
  concept can_sub_view = requires { sub_view(std::declval<It>(), std::declval<Sent>()); };
}

struct Sub {
   // for: bel::views::sub(beg, end)
   template<typename It, typename Sent>
   //requires ???
   constexpr auto
   operator() [[nodiscard]] (It&& it, Sent&& sent) const {
     return sub_view{std::forward<It>(it), std::forward<Sent>(sent)};
   }
};

// belleviews::sub() :
inline constexpr Sub sub;

} // namespace belleviews


namespace bel::views {
  // bel::views::sub() :
  inline constexpr belleviews::Sub sub;
}


// allows to use bel::subrange instead of belleviews::sub_view
namespace bel {
  template<std::input_or_output_iterator It, std::sentinel_for<It> Sent = It,
           belleviews::subrange_kind Kind = std::sized_sentinel_for<Sent, It>
                                              ? belleviews::subrange_kind::sized
                                              : belleviews::subrange_kind::unsized>
  requires (Kind == belleviews::subrange_kind::sized || !std::sized_sentinel_for<Sent, It>)
  // bel::subrange{} :
  using subrange = belleviews::sub_view<It, Sent, Kind>;
}

#endif // BELLESUB_HPP
