#ifndef BELLETRANSFORM_HPP
#define BELLETRANSFORM_HPP

#include "makeconstiterator.hpp"

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
      template<bool _Const>
        using _Base = _intern::maybe_const_t<_Const, V>;

      template<bool _Const>
        struct __iter_cat
        { };

      template<bool _Const>
        requires std::ranges::forward_range<_Base<_Const>>
        struct __iter_cat<_Const>
        {
        private:
          static auto
          _S_iter_cat()
          {
            using _Base = transform_view::_Base<_Const>;
            using _Res = std::invoke_result_t<F&, std::ranges::range_reference_t<_Base>>;
            if constexpr (std::is_lvalue_reference_v<_Res>)
              {
                using _Cat
                  = typename std::iterator_traits<std::ranges::iterator_t<_Base>>::iterator_category;
                if constexpr (std::derived_from<_Cat, std::contiguous_iterator_tag>)
                  return std::random_access_iterator_tag{};
                else
                  return _Cat{};
              }
            else
              return std::input_iterator_tag{};
          }
        public:
          using iterator_category = decltype(_S_iter_cat());
        };

      template<bool _Const>
        struct _Sentinel;

      template<bool _Const>
        struct _Iterator : __iter_cat<_Const>
        {
        private:
          using _Parent = _intern::maybe_const_t<_Const, transform_view>;
          using _Base = transform_view::_Base<_Const>;

          static auto
          _S_iter_concept()
          {
            if constexpr (std::ranges::random_access_range<_Base>)
              return std::random_access_iterator_tag{};
            else if constexpr (std::ranges::bidirectional_range<_Base>)
              return std::bidirectional_iterator_tag{};
            else if constexpr (std::ranges::forward_range<_Base>)
              return std::forward_iterator_tag{};
            else
              return std::input_iterator_tag{};
          }

          using _Base_iter = std::ranges::iterator_t<_Base>;

          _Base_iter _M_current = _Base_iter();
          _Parent* _M_parent = nullptr;

        public:
          using iterator_concept = decltype(_S_iter_concept());
          // iterator_category defined in __transform_view_iter_cat
          using value_type
            = std::remove_cvref_t<std::invoke_result_t<F&, std::ranges::range_reference_t<_Base>>>;
          using difference_type = std::ranges::range_difference_t<_Base>;

          _Iterator() requires std::default_initializable<_Base_iter> = default;

          constexpr
          _Iterator(_Parent* __parent, _Base_iter __current)
            : _M_current(std::move(__current)),
              _M_parent(__parent)
          { }

          constexpr
          _Iterator(_Iterator<!_Const> __i)
            requires _Const
              && std::convertible_to<std::ranges::iterator_t<V>, _Base_iter>
            : _M_current(std::move(__i._M_current)), _M_parent(__i._M_parent)
          { }

          constexpr const _Base_iter&
          base() const & noexcept
          { return _M_current; }

          constexpr _Base_iter
          base() &&
          { return std::move(_M_current); }

          constexpr decltype(auto)
          operator*() const
            noexcept(noexcept(std::invoke(*_M_parent->_M_fun, *_M_current)))
          { return std::invoke(*_M_parent->_M_fun, *_M_current); }

          constexpr _Iterator&
          operator++()
          {
            ++_M_current;
            return *this;
          }

          constexpr void
          operator++(int)
          { ++_M_current; }

          constexpr _Iterator
          operator++(int) requires std::ranges::forward_range<_Base>
          {
            auto __tmp = *this;
            ++*this;
            return __tmp;
          }

          constexpr _Iterator&
          operator--() requires std::ranges::bidirectional_range<_Base>
          {
            --_M_current;
            return *this;
          }

          constexpr _Iterator
          operator--(int) requires std::ranges::bidirectional_range<_Base>
          {
            auto __tmp = *this;
            --*this;
            return __tmp;
          }

          constexpr _Iterator&
          operator+=(difference_type __n) requires std::ranges::random_access_range<_Base>
          {
            _M_current += __n;
            return *this;
          }

          constexpr _Iterator&
          operator-=(difference_type __n) requires std::ranges::random_access_range<_Base>
          {
            _M_current -= __n;
            return *this;
          }

          constexpr decltype(auto)
          operator[](difference_type __n) const
            requires std::ranges::random_access_range<_Base>
          { return std::invoke(*_M_parent->_M_fun, _M_current[__n]); }

          friend constexpr bool
          operator==(const _Iterator& __x, const _Iterator& __y)
            requires std::equality_comparable<_Base_iter>
          { return __x._M_current == __y._M_current; }

          friend constexpr bool
          operator<(const _Iterator& __x, const _Iterator& __y)
            requires std::ranges::random_access_range<_Base>
          { return __x._M_current < __y._M_current; }

          friend constexpr bool
          operator>(const _Iterator& __x, const _Iterator& __y)
            requires std::ranges::random_access_range<_Base>
          { return __y < __x; }

          friend constexpr bool
          operator<=(const _Iterator& __x, const _Iterator& __y)
            requires std::ranges::random_access_range<_Base>
          { return !(__y < __x); }

          friend constexpr bool
          operator>=(const _Iterator& __x, const _Iterator& __y)
            requires std::ranges::random_access_range<_Base>
          { return !(__x < __y); }

#ifdef __cpp_lib_three_way_comparison
          friend constexpr auto
          operator<=>(const _Iterator& __x, const _Iterator& __y)
            requires std::ranges::random_access_range<_Base>
              && std::three_way_comparable<_Base_iter>
          { return __x._M_current <=> __y._M_current; }
#endif

          friend constexpr _Iterator
          operator+(_Iterator __i, difference_type __n)
            requires std::ranges::random_access_range<_Base>
          { return {__i._M_parent, __i._M_current + __n}; }

          friend constexpr _Iterator
          operator+(difference_type __n, _Iterator __i)
            requires std::ranges::random_access_range<_Base>
          { return {__i._M_parent, __i._M_current + __n}; }

          friend constexpr _Iterator
          operator-(_Iterator __i, difference_type __n)
            requires std::ranges::random_access_range<_Base>
          { return {__i._M_parent, __i._M_current - __n}; }

          // _GLIBCXX_RESOLVE_LIB_DEFECTS
          // 3483. transform_view::iterator's difference is overconstrained
          friend constexpr difference_type
          operator-(const _Iterator& __x, const _Iterator& __y)
            requires std::sized_sentinel_for<std::ranges::iterator_t<_Base>, std::ranges::iterator_t<_Base>>
          { return __x._M_current - __y._M_current; }

          friend constexpr decltype(auto)
          iter_move(const _Iterator& __i) noexcept(noexcept(*__i))
          {
            if constexpr (std::is_lvalue_reference_v<decltype(*__i)>)
              return std::move(*__i);
            else
              return *__i;
          }

          friend _Iterator<!_Const>;
          template<bool> friend struct _Sentinel;
        };

      template<bool _Const>
        struct _Sentinel
        {
        private:
          using _Parent = _intern::maybe_const_t<_Const, transform_view>;
          using _Base = transform_view::_Base<_Const>;

          template<bool _Const2>
            constexpr auto
            __distance_from(const _Iterator<_Const2>& __i) const
            { return _M_end - __i._M_current; }

          template<bool _Const2>
            constexpr bool
            __equal(const _Iterator<_Const2>& __i) const
            { return __i._M_current == _M_end; }

          std::ranges::sentinel_t<_Base> _M_end = std::ranges::sentinel_t<_Base>();

        public:
          _Sentinel() = default;

          constexpr explicit
          _Sentinel(std::ranges::sentinel_t<_Base> __end)
            : _M_end(__end)
          { }

          constexpr
          _Sentinel(_Sentinel<!_Const> __i)
            requires _Const
              && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<_Base>>
            : _M_end(std::move(__i._M_end))
          { }

          constexpr std::ranges::sentinel_t<_Base>
          base() const
          { return _M_end; }

          template<bool _Const2>
            requires std::sentinel_for<std::ranges::sentinel_t<_Base>,
                       std::ranges::iterator_t<_intern::maybe_const_t<_Const2, V>>>
            friend constexpr bool
            operator==(const _Iterator<_Const2>& __x, const _Sentinel& __y)
            { return __y.__equal(__x); }

          template<bool _Const2,
                   typename _Base2 = _intern::maybe_const_t<_Const2, V>>
            requires std::sized_sentinel_for<std::ranges::sentinel_t<_Base>, std::ranges::iterator_t<_Base2>>
            friend constexpr std::ranges::range_difference_t<_Base2>
            operator-(const _Iterator<_Const2>& __x, const _Sentinel& __y)
            { return -__y.__distance_from(__x); }

          template<bool _Const2,
                   typename _Base2 = _intern::maybe_const_t<_Const2, V>>
            requires std::sized_sentinel_for<std::ranges::sentinel_t<_Base>, std::ranges::iterator_t<_Base2>>
            friend constexpr std::ranges::range_difference_t<_Base2>
            operator-(const _Sentinel& __y, const _Iterator<_Const2>& __x)
            { return __y.__distance_from(__x); }

          friend _Sentinel<!_Const>;
        };

      V _M_base = V();
      [[no_unique_address]] _intern::box<F> _M_fun;

 public:
  transform_view() requires (std::default_initializable<V> && std::default_initializable<F>)
   = default;

  constexpr transform_view(V __base, F __fun)
   : _M_base(std::move(__base)), _M_fun(std::move(__fun)) {
  }

  constexpr V base() const& requires std::copy_constructible<V> { return _M_base ; }
  constexpr V base() && { return std::move(_M_base); }

  constexpr _Iterator<false> begin() {
    return _Iterator<false>{this, std::ranges::begin(_M_base)};
  }

  constexpr auto begin() const requires std::ranges::range<const V>
                                                    && std::regular_invocable<const F&,
                                                                              std::ranges::range_reference_t<const V>> {
    return make_const_iterator(_Iterator<true>{this, std::ranges::begin(_M_base)});
  }

  constexpr _Sentinel<false> end() {
    return _Sentinel<false>{std::ranges::end(_M_base)};
  }

  constexpr _Iterator<false> end() requires std::ranges::common_range<V> {
    return _Iterator<false>{this, std::ranges::end(_M_base)};
  }

  constexpr auto end() const requires std::ranges::range<const V>
                                                  && std::regular_invocable<const F&,
                                                                            std::ranges::range_reference_t<const V>> {
    return make_const_sentinel(_Sentinel<true>{std::ranges::end(_M_base)});
  }

  constexpr auto end() const requires std::ranges::common_range<const V>
                                                  && std::regular_invocable<const F&,
                                                                            std::ranges::range_reference_t<const V>> {
    return make_const_iterator(_Iterator<true>{this, std::ranges::end(_M_base)});
  }

  constexpr auto size() requires std::ranges::sized_range<V> {
    return std::ranges::size(_M_base);
  }
  constexpr auto size() const requires std::ranges::sized_range<const V> {
    return std::ranges::size(_M_base);
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

struct _Transform {
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

inline constexpr _Transform transform;

} // namespace belleviews

namespace bel::views {
  inline constexpr belleviews::_Transform transform;
}

#endif // BELLETRANSFORM_HPP
