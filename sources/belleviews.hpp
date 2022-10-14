//**********************************************************************
// C++20 and C++23 introduces view types that are 
// - highly error-prone and
// - not self-explanatory
// Because the following expected abilities of collections
// known from containers are
// BROKEN by std::views:
// - You can iterate over the elements when the collections are const
// - You can concurrently iterate to read
//   - You can use parallel STL algorithms
// - const is propagated (declaring the collection const makes the elements const)
// - cbegin() is provided and makes the elements temporarily const
//   - partially fixed with C++23
// - Elements declared with const auto& are const (broken with C++23)
// - Iterations that read over do not affect later behavior/validity
// - Copying a collection creates a collection with the same state
// - type const_iterator is available
//
// The main reason that all these guarantees are broken is performance.
// To avoid expensive begin(), begin() may be cached.
// As a consequence all the guarantees above may be broken.
// As most of the time views are only used once, this optimization is often worthless.
//
// This is an alternative approach for a view library that keeps all the knows guarantees.
// In some cases performance might become worse.
// However, on the other hand a couple of unexpected errors and UB is avoided.
//**********************************************************************
#ifndef BELLEVIEWS_HPP
#define BELLEVIEWS_HPP

#include "makeconstiterator.hpp"

namespace belleviews::_intern {

  // is_initializer_list
  template<typename _Tp>
  inline constexpr bool is_initializer_list = false;

  template<typename _Tp>
  inline constexpr bool is_initializer_list<std::initializer_list<_Tp>> = true;

  // simple_view
  template<typename Rg>
  concept simple_view = std::ranges::view<Rg> && std::ranges::range<const Rg>
                         && std::same_as<std::ranges::iterator_t<Rg>, std::ranges::iterator_t<const Rg>>
                         && std::same_as<std::ranges::sentinel_t<Rg>, std::ranges::sentinel_t<const Rg>>;

  // has_arrow
  template<typename It>
  concept has_arrow = std::input_iterator<It> &&
                      (std::is_pointer_v<It> || requires(It it) { it.operator->(); });

  template<typename _Tp, typename _Up>
  concept different_from = !std::same_as<std::remove_cvref_t<_Tp>, std::remove_cvref_t<_Up>>;

  // maybe_const
  template<bool _Const, typename _Tp>
    using maybe_const_t = std::conditional_t<_Const, const _Tp, _Tp>;

  // can_reference:
  template<typename _Tp>
    using with_ref = _Tp&;

  template<typename _Tp>
    concept can_reference = requires { typename with_ref<_Tp>; };


  // reference, iterator and sentinel types:
  // - partially come with C++23
  template<std::indirectly_readable It>
  using iter_const_reference_t = std::common_reference_t<const std::iter_value_t<It>&&, std::iter_reference_t<It>>;

  template<std::ranges::range R>
  using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;
  //template<input_iterator I>
  //using const_iterator = see below ;
  // Result: If I models constant-iterator , I. Otherwise, basic_const_iterator<I>.

  template<std::indirectly_readable It>
  using iter_const_rvalue_reference_t = std::common_reference_t<const std::iter_value_t<It>&&, std::iter_rvalue_reference_t<It>>;
  template<std::ranges::range R>
  using range_const_rvalue_reference_t = iter_const_rvalue_reference_t<std::ranges::iterator_t<R>>;

  template<std::ranges::range R>
  using const_iterator_t = const_iterator<std::ranges::iterator_t<R>>;

  //template<class S>
  //using const_sentinel = see below ;
  // Result: If S models input_iterator, const_iterator<S>. Otherwise, S.
  template<typename It>
  using const_sentinel = decltype(make_const_sentinel(std::declval<It>()));

  template<std::ranges::range R>
  using const_sentinel_t = const_sentinel<std::ranges::sentinel_t<R>>;
  
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

#include "belletake.hpp"
#include "belledrop.hpp"
#ifdef BEL_BORROWED
#include "bellefilterborrowed.hpp"
#else
#include "bellefilter.hpp"
#endif
#include "bellesub.hpp"
#include "belleall.hpp"

#endif // BELLEVIEWS_HPP
