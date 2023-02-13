// <belletransform.hpp> -*- C++ -*-
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

#ifndef BELLETRANSFORM_HPP
#define BELLETRANSFORM_HPP

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::transform_view
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

template<std::ranges::input_range V, std::copy_constructible F>
requires std::ranges::view<V> && std::is_object_v<F>
          && std::regular_invocable<F&, std::ranges::range_reference_t<V>>
          && _intern::can_reference<std::invoke_result_t<F&, std::ranges::range_reference_t<V>>>
class transform_view : public std::ranges::view_interface<transform_view<V, F>>
{
 private:
      template<bool ConstT>
        using Base = _intern::maybe_const_t<ConstT, V>;

      template<bool ConstT>
        struct __iter_cat
        { };

      template<bool ConstT>
        requires std::ranges::forward_range<Base<ConstT>>
        struct __iter_cat<ConstT>
        {
        private:
          static auto
          _s_iter_cat()
          {
            using Base = transform_view::Base<ConstT>;
            using Res = std::invoke_result_t<F&, std::ranges::range_reference_t<Base>>;
            if constexpr (std::is_lvalue_reference_v<Res>)
              {
                using Cat
                  = typename std::iterator_traits<std::ranges::iterator_t<Base>>::iterator_category;
                if constexpr (std::derived_from<Cat, std::contiguous_iterator_tag>)
                  return std::random_access_iterator_tag{};
                else
                  return Cat{};
              }
            else
              return std::input_iterator_tag{};
          }
        public:
          using iterator_category = decltype(_s_iter_cat());
        };

      template<bool ConstT>
        struct Sentinel;

      template<bool ConstT>
        struct Iterator : __iter_cat<ConstT>
        {
        private:
          using Parent = _intern::maybe_const_t<ConstT, transform_view>;
          using Base = transform_view::Base<ConstT>;

          static auto
          _s_iter_concept()
          {
            if constexpr (std::ranges::random_access_range<Base>)
              return std::random_access_iterator_tag{};
            else if constexpr (std::ranges::bidirectional_range<Base>)
              return std::bidirectional_iterator_tag{};
            else if constexpr (std::ranges::forward_range<Base>)
              return std::forward_iterator_tag{};
            else
              return std::input_iterator_tag{};
          }

          using Base_iter = std::ranges::iterator_t<Base>;

          Base_iter _m_current = Base_iter();
          Parent* _m_parent = nullptr;

        public:
          using iterator_concept = decltype(_s_iter_concept());
          // iterator_category defined in __transform_view_iter_cat
          using value_type
            = std::remove_cvref_t<std::invoke_result_t<F&, std::ranges::range_reference_t<Base>>>;
          using difference_type = std::ranges::range_difference_t<Base>;

          Iterator() requires std::default_initializable<Base_iter> = default;

          constexpr
          Iterator(Parent* __parent, Base_iter __current)
            : _m_current(std::move(__current)),
              _m_parent(__parent)
          { }

          constexpr
          Iterator(Iterator<!ConstT> __i)
            requires ConstT
              && std::convertible_to<std::ranges::iterator_t<V>, Base_iter>
            : _m_current(std::move(__i._m_current)), _m_parent(__i._m_parent)
          { }

          constexpr const Base_iter&
          base() const & noexcept
          { return _m_current; }

          constexpr Base_iter
          base() &&
          { return std::move(_m_current); }

          constexpr decltype(auto)
          operator*() const
            noexcept(noexcept(std::invoke(*_m_parent->_m_fun, *_m_current)))
          { return std::invoke(*_m_parent->_m_fun, *_m_current); }

          constexpr Iterator&
          operator++()
          {
            ++_m_current;
            return *this;
          }

          constexpr void
          operator++(int)
          { ++_m_current; }

          constexpr Iterator
          operator++(int) requires std::ranges::forward_range<Base>
          {
            auto __tmp = *this;
            ++*this;
            return __tmp;
          }

          constexpr Iterator&
          operator--() requires std::ranges::bidirectional_range<Base>
          {
            --_m_current;
            return *this;
          }

          constexpr Iterator
          operator--(int) requires std::ranges::bidirectional_range<Base>
          {
            auto __tmp = *this;
            --*this;
            return __tmp;
          }

          constexpr Iterator&
          operator+=(difference_type __n) requires std::ranges::random_access_range<Base>
          {
            _m_current += __n;
            return *this;
          }

          constexpr Iterator&
          operator-=(difference_type __n) requires std::ranges::random_access_range<Base>
          {
            _m_current -= __n;
            return *this;
          }

          constexpr decltype(auto)
          operator[](difference_type __n) const
            requires std::ranges::random_access_range<Base>
          { return std::invoke(*_m_parent->_m_fun, _m_current[__n]); }

          friend constexpr bool
          operator==(const Iterator& __x, const Iterator& __y)
            requires std::equality_comparable<Base_iter>
          { return __x._m_current == __y._m_current; }

          friend constexpr bool
          operator<(const Iterator& __x, const Iterator& __y)
            requires std::ranges::random_access_range<Base>
          { return __x._m_current < __y._m_current; }

          friend constexpr bool
          operator>(const Iterator& __x, const Iterator& __y)
            requires std::ranges::random_access_range<Base>
          { return __y < __x; }

          friend constexpr bool
          operator<=(const Iterator& __x, const Iterator& __y)
            requires std::ranges::random_access_range<Base>
          { return !(__y < __x); }

          friend constexpr bool
          operator>=(const Iterator& __x, const Iterator& __y)
            requires std::ranges::random_access_range<Base>
          { return !(__x < __y); }

#ifdef __cpp_lib_three_way_comparison
          friend constexpr auto
          operator<=>(const Iterator& __x, const Iterator& __y)
            requires std::ranges::random_access_range<Base>
              && std::three_way_comparable<Base_iter>
          { return __x._m_current <=> __y._m_current; }
#endif

          friend constexpr Iterator
          operator+(Iterator __i, difference_type __n)
            requires std::ranges::random_access_range<Base>
          { return {__i._m_parent, __i._m_current + __n}; }

          friend constexpr Iterator
          operator+(difference_type __n, Iterator __i)
            requires std::ranges::random_access_range<Base>
          { return {__i._m_parent, __i._m_current + __n}; }

          friend constexpr Iterator
          operator-(Iterator __i, difference_type __n)
            requires std::ranges::random_access_range<Base>
          { return {__i._m_parent, __i._m_current - __n}; }

          // _GLIBCXX_RESOLVE_LIB_DEFECTS
          // 3483. transform_view::iterator's difference is overconstrained
          friend constexpr difference_type
          operator-(const Iterator& __x, const Iterator& __y)
            requires std::sized_sentinel_for<std::ranges::iterator_t<Base>, std::ranges::iterator_t<Base>>
          { return __x._m_current - __y._m_current; }

          friend constexpr decltype(auto)
          iter_move(const Iterator& __i) noexcept(noexcept(*__i))
          {
            if constexpr (std::is_lvalue_reference_v<decltype(*__i)>)
              return std::move(*__i);
            else
              return *__i;
          }

          friend Iterator<!ConstT>;
          template<bool> friend struct Sentinel;
        };

      template<bool ConstT>
        struct Sentinel
        {
        private:
          using Parent = _intern::maybe_const_t<ConstT, transform_view>;
          using Base = transform_view::Base<ConstT>;

          template<bool Const2T>
            constexpr auto
            __distance_from(const Iterator<Const2T>& __i) const
            { return _m_end - __i._m_current; }

          template<bool Const2T>
            constexpr bool
            __equal(const Iterator<Const2T>& __i) const
            { return __i._m_current == _m_end; }

          std::ranges::sentinel_t<Base> _m_end = std::ranges::sentinel_t<Base>();

        public:
          Sentinel() = default;

          constexpr explicit
          Sentinel(std::ranges::sentinel_t<Base> __end)
            : _m_end(__end)
          { }

          constexpr
          Sentinel(Sentinel<!ConstT> __i)
            requires ConstT
              && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base>>
            : _m_end(std::move(__i._m_end))
          { }

          constexpr std::ranges::sentinel_t<Base>
          base() const
          { return _m_end; }

          template<bool Const2T>
            requires std::sentinel_for<std::ranges::sentinel_t<Base>,
                       std::ranges::iterator_t<_intern::maybe_const_t<Const2T, V>>>
            friend constexpr bool
            operator==(const Iterator<Const2T>& __x, const Sentinel& __y)
            { return __y.__equal(__x); }

          template<bool Const2T,
                   typename Base2 = _intern::maybe_const_t<Const2T, V>>
            requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base2>>
            friend constexpr std::ranges::range_difference_t<Base2>
            operator-(const Iterator<Const2T>& __x, const Sentinel& __y)
            { return -__y.__distance_from(__x); }

          template<bool Const2T,
                   typename Base2 = _intern::maybe_const_t<Const2T, V>>
            requires std::sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base2>>
            friend constexpr std::ranges::range_difference_t<Base2>
            operator-(const Sentinel& __y, const Iterator<Const2T>& __x)
            { return __y.__distance_from(__x); }

          friend Sentinel<!ConstT>;
        };

      V _m_base = V();
      [[no_unique_address]] _intern::SemiregBox<F> _m_fun;

 public:
  transform_view() requires (std::default_initializable<V> && std::default_initializable<F>)
   = default;

  constexpr transform_view(V __base, F __fun)
   : _m_base(std::move(__base)), _m_fun(std::move(__fun)) {
  }

  constexpr V base() const& requires std::copy_constructible<V> { return _m_base ; }
  constexpr V base() && { return std::move(_m_base); }

  constexpr Iterator<false> begin() {
    return Iterator<false>{this, std::ranges::begin(_m_base)};
  }

  constexpr auto begin() const requires std::ranges::range<const V>
                                                    && std::regular_invocable<const F&,
                                                                              std::ranges::range_reference_t<const V>> {
    return std::make_const_iterator(Iterator<true>{this, std::ranges::begin(_m_base)});
  }

  constexpr Sentinel<false> end() {
    return Sentinel<false>{std::ranges::end(_m_base)};
  }

  constexpr Iterator<false> end() requires std::ranges::common_range<V> {
    return Iterator<false>{this, std::ranges::end(_m_base)};
  }

  constexpr auto end() const requires std::ranges::range<const V>
                                                  && std::regular_invocable<const F&,
                                                                            std::ranges::range_reference_t<const V>> {
    return std::make_const_sentinel(Sentinel<true>{std::ranges::end(_m_base)});
  }

  constexpr auto end() const requires std::ranges::common_range<const V>
                                                  && std::regular_invocable<const F&,
                                                                            std::ranges::range_reference_t<const V>> {
    return std::make_const_iterator(Iterator<true>{this, std::ranges::end(_m_base)});
  }

  constexpr auto size() requires std::ranges::sized_range<V> {
    return std::ranges::size(_m_base);
  }
  constexpr auto size() const requires std::ranges::sized_range<const V> {
    return std::ranges::size(_m_base);
  }
};

template<typename R, typename F>
transform_view(R&&, F) -> transform_view<std::views::all_t<R>, F>;

} // namespace belleviews

// NO BORROWED VIEW:
//template<typename V, typename Func>
//inline constexpr bool std::ranges::enable_borrowed_range<belleviews::transform_view<V, Func>> = std::ranges::enable_borrowed_range<V>;


//*************************************************************
// belleviews::transform()
// bel::views::transform()
// 
// A C++ transform_view adaptor for the belleviews::transform_view
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg, typename Func>
  concept can_transform_view = requires { transform_view(std::declval<Rg>(), std::declval<Func>()); };
}

struct Transform {
   // for:  bel::views::transform(rg, func)
   template<std::ranges::viewable_range Rg, typename Func>
   requires _intern::can_transform_view<Rg, Func>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg, Func&& func) const {
     return transform_view{std::forward<Rg>(rg), std::forward<Func>(func)};
   }

   // for:  rg | belleviews::transform(func)
   template<typename T>
   struct PartialTransform {
     T func;
   };

   template<typename Func>
   constexpr auto
   operator() [[nodiscard]] (Func func) const {
     return PartialTransform<Func>{func};
   }

   template<typename Rg, typename Func>
   friend constexpr auto
   operator| (Rg&& rg, PartialTransform<Func> pd) {
     return transform_view{std::forward<Rg>(rg), pd.func};
   }
};

// belleviews::transform() :
inline constexpr Transform transform;

} // namespace belleviews

namespace bel::views {
  // bel::views::transform() :
  inline constexpr belleviews::Transform transform;
}

#endif // BELLETRANSFORM_HPP
