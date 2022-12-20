// <belleall.hpp> -*- C++ -*-
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

#ifndef BELLEALL_HPP
#define BELLEALL_HPP

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::ref_view
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

template<std::ranges::range Rg>
requires std::is_object_v<Rg>
class ref_view : public std::ranges::view_interface<ref_view<Rg>>
{
 private:
  Rg* rgPtr;

  static void _S_fun(Rg&); // not defined
  static void _S_fun(Rg&&) = delete;

 public:
  template<_intern::different_from<ref_view> T>
  requires std::convertible_to<T, Rg&> && requires { _S_fun(std::declval<T>()); }
  constexpr ref_view(T&& __t) noexcept(noexcept(static_cast<Rg&>(std::declval<T>())))
   : rgPtr(std::addressof(static_cast<Rg&>(std::forward<T>(__t)))) {
  }

  constexpr Rg& base() const {
    return *rgPtr; 
  }

  // begin():
  constexpr std::ranges::iterator_t<Rg> begin() { 
    return std::ranges::begin(*rgPtr); 
  }
  constexpr auto begin() const { 
    return std::make_const_iterator(std::ranges::begin(*rgPtr)); 
  }

  // end():
  constexpr std::ranges::sentinel_t<Rg> end() { 
    return std::ranges::end(*rgPtr); 
  }
  constexpr auto end() const { 
    return std::make_const_iterator(std::ranges::end(*rgPtr)); 
  }

  constexpr bool empty() const requires requires { std::ranges::empty(*rgPtr); } { 
    return std::ranges::empty(*rgPtr); 
  }

  constexpr auto size() const requires std::ranges::sized_range<Rg> {
    return std::ranges::size(*rgPtr); 
  }

  constexpr auto data() const requires std::ranges::contiguous_range<Rg> {
    return std::ranges::data(*rgPtr);
  }
};

template<typename Rg>
ref_view(Rg&) -> ref_view<Rg>;

}// namespace belleviews

// always borrowed (as with std ref_view):
template<typename Rg>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::ref_view<Rg>> = true;


//*************************************************************
// class belleviews::owning_view
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

template<std::ranges::range Rg>
requires std::movable<Rg> && (!_intern::is_initializer_list<std::remove_cv_t<Rg>>)
class owning_view : public std::ranges::view_interface<owning_view<Rg>>
{
 private:
   Rg rg = Rg();

 public:
   owning_view() requires std::default_initializable<Rg> = default;

   constexpr owning_view(Rg&& __t) noexcept(std::is_nothrow_move_constructible_v<Rg>)
    : rg(std::move(__t)) {
   }

   owning_view(owning_view&&) = default;
   owning_view& operator=(owning_view&&) = default;

   constexpr Rg& base() & noexcept { return rg; }
   constexpr const Rg& base() const& noexcept { return rg; }

   constexpr Rg&& base() && noexcept { return std::move(rg); }
   constexpr const Rg&& base() const&& noexcept { return std::move(rg); }

   // begin():
   constexpr std::ranges::iterator_t<Rg> begin() {
     return std::ranges::begin(rg);
   }

   constexpr auto begin() const requires std::ranges::range<const Rg> {
     return std::make_const_iterator(std::ranges::begin(rg)); 
   }

   // end():
   constexpr std::ranges::sentinel_t<Rg> end() {
     return std::ranges::end(rg);
   }
   constexpr auto end() const requires std::ranges::range<const Rg> {
     return std::make_const_iterator(std::ranges::end(rg));
   }

   constexpr bool empty() requires requires { std::ranges::empty(rg); } {
     return std::ranges::empty(rg);
   }

   // empty() and size():
   constexpr bool empty() const requires requires { std::ranges::empty(rg); } {
     return std::ranges::empty(rg);
   }

   constexpr auto size() requires std::ranges::sized_range<Rg> {
     return std::ranges::size(rg);
   }

   constexpr auto size() const requires std::ranges::sized_range<const Rg> {
     return std::ranges::size(rg);
   }

   // data():
   constexpr auto data() requires std::ranges::contiguous_range<Rg> {
     return std::ranges::data(rg);
   }

   constexpr auto data() const requires std::ranges::contiguous_range<const Rg> {
     return std::ranges::data(rg);
   }
};

} // namespace belleviews

// borrowed if the underlying range is borrowed:
template<typename Rg>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::owning_view<Rg>> = std::ranges::enable_borrowed_range<Rg>;
  


//*************************************************************
// belleviews::all()
// bel::views::all()
// bel::views::all_t
// 
// A C++ view adaptor for different belleviews views
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg>
  concept can_ref_view = requires { ref_view(std::declval<Rg>()); };

  template<typename Rg>
  concept can_owning_view = requires { owning_view(std::declval<Rg>()); };
}

struct All {
  // for:  bel::views::all(rg)
  template<std::ranges::viewable_range Rg>
  requires std::ranges::view<Rg> || _intern::can_ref_view<Rg> || _intern::can_owning_view<Rg>
  constexpr auto
  operator() [[nodiscard]] (Rg&& rg) const {
    if constexpr (std::ranges::view<std::decay_t<Rg>>) {
      return std::forward<Rg>(rg);
    }
    else if constexpr (_intern::can_ref_view<Rg>) {
      return ref_view{std::forward<Rg>(rg)};
    }
    else {
      return owning_view{std::forward<Rg>(rg)};
    }
  }
};


// belleviews::all() :
inline constexpr All all;

// belleviews::all_t :
template<std::ranges::viewable_range Rg>
  using all_t = decltype(all(std::declval<Rg>()));

} // namespace belleviews


namespace bel::views {

  // bel::views::all() :
  inline constexpr belleviews::All all;

  // bel::views::all_t :
  template<std::ranges::viewable_range Rg>
    using all_t = decltype(all(std::declval<Rg>()));
}

#endif // BELLEALL_HPP
