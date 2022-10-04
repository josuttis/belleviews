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

namespace belleviews {

namespace _intern {
  template<typename Rg>
  concept simple_view = std::ranges::view<Rg> && std::ranges::range<const Rg>
                         && std::same_as<std::ranges::iterator_t<Rg>, std::ranges::iterator_t<const Rg>>
                         && std::same_as<std::ranges::sentinel_t<Rg>, std::ranges::sentinel_t<const Rg>>;

  template<typename It>
  concept has_arrow = std::input_iterator<It> &&
                      (std::is_pointer_v<It> || requires(It it) { it.operator->(); });

  template<bool _Const, typename _Tp>
    using maybe_const_t = std::conditional_t<_Const, const _Tp, _Tp>;

  // might be missing:
  template<std::indirectly_readable It>
  using iter_const_reference_t = std::common_reference_t<const std::iter_value_t<It>&&, std::iter_reference_t<It>>;
  template<std::ranges::range R>
  using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;
  //template<input_iterator I>
  //using const_iterator = see below ;
  //  //1 Result: If I models constant-iterator , I. Otherwise, basic_const_iterator<I>.
  template<std::ranges::range R>
  using const_iterator_t = const_iterator<std::ranges::iterator_t<R>>;
}

} // namespace belleviews

#include "belletake.hpp"
#include "belledrop.hpp"
#include "bellefilter.hpp"


#endif // BELLEVIEWS_HPP
