#ifndef BELLETRANSFORM_HPP
#define BELLETRANSFORM_HPP

#include "makeconstiterator.hpp"

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::transform_view
// 
// A C++ transform_view
// with the following benefits compared to C++ standard views
// - Always propagates const
// Because
// - This view yields const iterators when it is const
// OPEN/TODO:
// - ...
//*************************************************************
namespace belleviews {

  template<std::ranges::input_range _Vp, std::copy_constructible _Fp>
    requires std::ranges::view<_Vp> && std::is_object_v<_Fp>
      && std::regular_invocable<_Fp&, std::ranges::range_reference_t<_Vp>>
      && _intern::can_reference<std::invoke_result_t<_Fp&,
							std::ranges::range_reference_t<_Vp>>>
    class transform_view : public std::ranges::view_interface<transform_view<_Vp, _Fp>>
    {
    private:
      template<bool _Const>
	using _Base = _intern::maybe_const_t<_Const, _Vp>;

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
	    using _Res = std::invoke_result_t<_Fp&, std::ranges::range_reference_t<_Base>>;
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
	    = std::remove_cvref_t<std::invoke_result_t<_Fp&, std::ranges::range_reference_t<_Base>>>;
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
	      && std::convertible_to<std::ranges::iterator_t<_Vp>, _Base_iter>
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
	    noexcept(noexcept(std::__invoke(*_M_parent->_M_fun, *_M_current)))
	  { return std::__invoke(*_M_parent->_M_fun, *_M_current); }

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
	  { return std::__invoke(*_M_parent->_M_fun, _M_current[__n]); }

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
	      && std::convertible_to<std::ranges::sentinel_t<_Vp>, std::ranges::sentinel_t<_Base>>
	    : _M_end(std::move(__i._M_end))
	  { }

	  constexpr std::ranges::sentinel_t<_Base>
	  base() const
	  { return _M_end; }

	  template<bool _Const2>
	    requires std::sentinel_for<std::ranges::sentinel_t<_Base>,
		       std::ranges::iterator_t<_intern::maybe_const_t<_Const2, _Vp>>>
	    friend constexpr bool
	    operator==(const _Iterator<_Const2>& __x, const _Sentinel& __y)
	    { return __y.__equal(__x); }

	  template<bool _Const2,
		   typename _Base2 = _intern::maybe_const_t<_Const2, _Vp>>
	    requires std::sized_sentinel_for<std::ranges::sentinel_t<_Base>, std::ranges::iterator_t<_Base2>>
	    friend constexpr std::ranges::range_difference_t<_Base2>
	    operator-(const _Iterator<_Const2>& __x, const _Sentinel& __y)
	    { return -__y.__distance_from(__x); }

	  template<bool _Const2,
		   typename _Base2 = _intern::maybe_const_t<_Const2, _Vp>>
	    requires std::sized_sentinel_for<std::ranges::sentinel_t<_Base>, std::ranges::iterator_t<_Base2>>
	    friend constexpr std::ranges::range_difference_t<_Base2>
	    operator-(const _Sentinel& __y, const _Iterator<_Const2>& __x)
	    { return __y.__distance_from(__x); }

	  friend _Sentinel<!_Const>;
	};

      _Vp _M_base = _Vp();
      [[no_unique_address]] _intern::box<_Fp> _M_fun;

    public:
      transform_view() requires (std::default_initializable<_Vp>
				 && std::default_initializable<_Fp>)
	= default;

      constexpr
      transform_view(_Vp __base, _Fp __fun)
	: _M_base(std::move(__base)), _M_fun(std::move(__fun))
      { }

      constexpr _Vp
      base() const& requires std::copy_constructible<_Vp>
      { return _M_base ; }

      constexpr _Vp
      base() &&
      { return std::move(_M_base); }

      constexpr _Iterator<false>
      begin()
      { return _Iterator<false>{this, std::ranges::begin(_M_base)}; }

      constexpr _Iterator<true>
      begin() const
	requires std::ranges::range<const _Vp>
	  && std::regular_invocable<const _Fp&, std::ranges::range_reference_t<const _Vp>>
      { return _Iterator<true>{this, std::ranges::begin(_M_base)}; }

      constexpr _Sentinel<false>
      end()
      { return _Sentinel<false>{std::ranges::end(_M_base)}; }

      constexpr _Iterator<false>
      end() requires std::ranges::common_range<_Vp>
      { return _Iterator<false>{this, std::ranges::end(_M_base)}; }

      constexpr _Sentinel<true>
      end() const
	requires std::ranges::range<const _Vp>
	  && std::regular_invocable<const _Fp&, std::ranges::range_reference_t<const _Vp>>
      { return _Sentinel<true>{std::ranges::end(_M_base)}; }

      constexpr _Iterator<true>
      end() const
	requires std::ranges::common_range<const _Vp>
	  && std::regular_invocable<const _Fp&, std::ranges::range_reference_t<const _Vp>>
      { return _Iterator<true>{this, std::ranges::end(_M_base)}; }

      constexpr auto
      size() requires std::ranges::sized_range<_Vp>
      { return std::ranges::size(_M_base); }

      constexpr auto
      size() const requires std::ranges::sized_range<const _Vp>
      { return std::ranges::size(_M_base); }
    };

  template<typename _Range, typename _Fp>
    transform_view(_Range&&, _Fp) -> transform_view<std::views::all_t<_Range>, _Fp>;
} // namespace belleviews


//*************************************************************
// belleviews::transform()
// bel::views::transform()
// 
// A C++ transform_view adaptor for the belleviews::transform_view
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg, typename Pred>
  concept can_transform_view = requires { transform_view(std::declval<Rg>(), std::declval<Pred>()); };
}

struct _Transform {
   // for:  bel::views::transform(rg, pred)
   template<std::ranges::viewable_range Rg, typename Pred>
   requires _intern::can_transform_view<Rg, Pred>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg, Pred&& pred) const {
     return transform_view{std::forward<Rg>(rg), std::forward<Pred>(pred)};
   }

   // for:  rg | belleviews::transform(pred)
   template<typename T>
   struct PartialTransform {
     T pred;
   };

   template<typename Pred>
   constexpr auto
   operator() [[nodiscard]] (Pred pred) const {
     return PartialTransform<Pred>{pred};
   }

   template<typename Rg, typename Pred>
   friend constexpr auto
   operator| (Rg&& rg, PartialTransform<Pred> pd) {
     return transform_view{std::forward<Rg>(rg), pd.pred};
   }
};

inline constexpr _Transform transform;

} // namespace belleviews

namespace bel::views {
  inline constexpr belleviews::_Transform transform;
}

#endif // BELLETRANSFORM_HPP
