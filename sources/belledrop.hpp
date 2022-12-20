// <belledrop.hpp> -*- C++ -*-
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

#ifndef BELLEDROP_HPP
#define BELLEDROP_HPP

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::drop_view
// 
// A C++ view
// with the following benefits compared to C++ standard views
// - Always propagates const
// Because
// - This view yields const iterators when it is const
// OPEN/TODO:
// - ...
//*************************************************************
namespace belleviews {

template<std::ranges::view V>
class drop_view : public std::ranges::view_interface<drop_view<V>>
{
 private:
  V base_ = V();
  std::ranges::range_difference_t<V> count_ = 0;
 public:
  drop_view() requires std::default_initializable<V> = default;

  constexpr drop_view(V v, std::ranges::range_difference_t<V> c)
   : base_(std::move(v)), count_{c} {
      assert(c >= 0);
  }

  constexpr V base() const& requires std::copy_constructible<V> { return base_; }
  constexpr V base() && { return std::move(base_); }

  constexpr auto begin()
  { //requires (!(_intern::simple_view<V> && std::ranges::random_access_range<const V> && std::ranges::sized_range<const V>)) {
    return std::ranges::next(std::ranges::begin(base_), count_,
                             std::ranges::end(base_));
  }
  constexpr auto begin() const
  { //requires std::ranges::random_access_range<const V> && std::ranges::sized_range<const V> {
    return std::make_const_iterator(std::ranges::next(std::ranges::begin(base_), count_,
                                                      std::ranges::end(base_)));
  }
  constexpr auto end()
  //requires (!_intern::simple_view<V>) {
  {
    return std::ranges::end(base_);
  }
  constexpr auto end() const
  //requires std::ranges::range<const V> {
  {
    return std::make_const_sentinel(std::ranges::end(base_));
  }

  constexpr auto size()
  requires std::ranges::sized_range<V> {
    const auto s = std::ranges::size(base_);
    const auto c = static_cast<decltype(s)>(count_);
    return s < c ? 0 : s - c;
  }
  constexpr auto size() const
  requires std::ranges::sized_range<const V> {
    const auto s = std::ranges::size(base_);
    const auto c = static_cast<decltype(s)>(count_);
    return s < c ? 0 : s - c;
  }
};

template<typename R>
drop_view(R&&, std::ranges::range_difference_t<R>) -> drop_view<std::views::all_t<R>>;

} // namespace belleviews

// borrowed if underlying range is borrowed (as with std drop_view):
template<typename Rg>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::drop_view<Rg>> = std::ranges::enable_borrowed_range<Rg>;


//*************************************************************
// belleviews::drop()
// bel::views::drop()
// 
// A C++ drop_view adaptor for the belleviews::drop_view
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg, typename DiffT>
  concept can_drop_view = requires { drop_view(std::declval<Rg>(), std::declval<DiffT>()); };
}

struct Drop {
   // for:  bel::views::drop(rg, 2)
   template<std::ranges::viewable_range Rg, typename DiffT = std::ranges::range_difference_t<Rg>>
   requires _intern::can_drop_view<Rg, DiffT>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg, DiffT diff) const {
     return drop_view{std::forward<Rg>(rg), diff};
   }

   // for:  rg | bel::views::drop(2)
   template<typename T>
   struct PartialDrop {
     T diff;
   };

   template<typename DiffT>
   constexpr auto
   operator() [[nodiscard]] (DiffT diff) const {
     return PartialDrop<DiffT>{diff};
   }

   template<typename Rg, typename DiffT>
   friend constexpr auto
   operator| (Rg&& rg, PartialDrop<DiffT> pd) {
     return drop_view{std::forward<Rg>(rg), pd.diff};
   }
};

// belleviews::drop() :
inline constexpr Drop drop;

}// namespace belleviews


namespace bel::views {
  // bel::views::drop() :
  inline constexpr belleviews::Drop drop;
}

#endif // BELLEDROP_HPP
