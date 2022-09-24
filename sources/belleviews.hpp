#include "makeconstiterator.hpp"

#ifndef BELLEVIEWS_HPP
#define BELLEVIEWS_HPP

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


#include <concepts>
#include <ranges>
#include <cassert>

namespace belleviews {

namespace _intern {
  template<typename Rg>
  concept simple_view = std::ranges::view<Rg> && std::ranges::range<const Rg>
                         && std::same_as<std::ranges::iterator_t<Rg>, std::ranges::iterator_t<const Rg>>
                         && std::same_as<std::ranges::sentinel_t<Rg>, std::ranges::sentinel_t<const Rg>>;
}

template<std::ranges::view V>
class drop_view : public std::ranges::view_interface<drop_view<V>>
{
 private:
  V base_ = V();
  std::ranges::range_difference_t<V> count_ = 0;
 public:
  drop_view() requires std::default_initializable<V> = default;

  constexpr drop_view(V b, std::ranges::range_difference_t<V> c)
   : base_(std::move(b)), count_{c} {
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
    return make_const_iterator(std::ranges::next(std::ranges::begin(base_), count_,
                                                 std::ranges::end(base_)));
  }
  constexpr auto end()
  requires (!_intern::simple_view<V>) {
    return std::ranges::end(base_);
  }
  constexpr auto end() const
  requires std::ranges::range<const V> {
    return std::ranges::end(base_);
    //return make_const_iterator(std::ranges::end(base_));
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

template<typename _Tp>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::drop_view<_Tp>> = std::ranges::enable_borrowed_range<_Tp>;

namespace belleviews {

namespace _intern {
  template<typename Rg, typename DiffT>
  concept can_drop_view = requires { drop_view(std::declval<Rg>(), std::declval<DiffT>()); };
}

struct _Drop {
   // for: belleviews::drop_view{coll, 2}
   template<std::ranges::viewable_range Rg, typename DiffT = std::ranges::range_difference_t<Rg>>
   requires _intern::can_drop_view<Rg, DiffT>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg, DiffT diff) const {
     return drop_view{std::forward<Rg>(rg), diff};
   }

   // for: coll | belleviews::drop(2)
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

inline constexpr _Drop drop;

} // namespace belleviews

namespace bel::views {
  //using drop = belleviews::drop;
  inline constexpr belleviews::_Drop drop;
}

#endif // BELLEVIEWS_HPP
