#ifndef BELLETAKE_HPP
#define BELLETAKE_HPP

#include "makeconstiterator.hpp"

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::take_view
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

  /* STD:
namespace std::ranges {
  template<view V>
  class take_view : public view_interface<take_view<V>> {
  private:
    V base_ = V(); // exposition only
    range_difference_t<V> count_ = 0; // exposition only
    // 26.7.10.3, class template take_view::sentinel
    template<bool> class sentinel ; // exposition only
  public:
  take_view() requires default_initializable<V> = default;
  constexpr take_view(V base, range_difference_t<V> count);

  constexpr V base() const & requires copy_constructible<V> { return base_; }
  constexpr V base() && { return std::move(base_); }

  constexpr auto begin() requires (!simple-view <V>) {
    if constexpr (sized_range<V>) {
      if constexpr (random_access_range<V>) {
        return ranges::begin(base_);
      } else {
        auto sz = range_difference_t<V>(size());
        return counted_iterator(ranges::begin(base_), sz);
      }
    } else {
      return counted_iterator(ranges::begin(base_), count_);
    }
  }
  constexpr auto begin() const requires range<const V> {
    if constexpr (sized_range<const V>) {
      if constexpr (random_access_range<const V>) {
        return ranges::begin(base_);
      } else {
        auto sz = range_difference_t<const V>(size());
        return counted_iterator(ranges::begin(base_), sz);
      }
    } else {
      return counted_iterator(ranges::begin(base_), count_);
    }
  }
  constexpr auto end() requires (!simple-view <V>) {
    if constexpr (sized_range<V>) {
      if constexpr (random_access_range<V>)
        return ranges::begin(base_) + range_difference_t<V>(size());
      else
        return default_sentinel;
    } else {
      return sentinel <false>{ranges::end(base_)};
    }
  }
  constexpr auto end() const requires range<const V> {
    if constexpr (sized_range<const V>) {
      if constexpr (random_access_range<const V>)
        return ranges::begin(base_) + range_difference_t<const V>(size());
      else
        return default_sentinel;
    } else {
      return sentinel <true>{ranges::end(base_)};
    }
  }
  constexpr auto size() requires sized_range<V> {
    auto n = ranges::size(base_);
    return ranges::min(n, static_cast<decltype(n)>(count_));
  }
  constexpr auto size() const requires sized_range<const V> {
    auto n = ranges::size(base_);
    return ranges::min(n, static_cast<decltype(n)>(count_));
  }
};
template<class R>
take_view(R&&, range_difference_t<R>) -> take_view<views::all_t<R>>;
}

namespace std::ranges {
template<view V>
template<bool Const>
class take_view<V>::sentinel {
  private:
    using Base = maybe-const <Const, V>; // exposition only
    template<bool OtherConst>
    using CI = counted_iterator<iterator_t<maybe-const <OtherConst, V>>>; // exposition only
    sentinel_t<Base > end_ = sentinel_t<Base >(); // exposition only
  public:
    sentinel () = default;
    constexpr explicit sentinel (sentinel_t<Base > end);
    constexpr sentinel (sentinel <!Const> s)
      requires Const && convertible_to<sentinel_t<V>, sentinel_t<Base >>;
    constexpr sentinel_t<Base > base() const;
    friend constexpr bool operator==(const CI <Const>& y, const sentinel & x);
    template<bool OtherConst = !Const>
      requires sentinel_for<sentinel_t<Base >, iterator_t<maybe-const <OtherConst, V>>>
      friend constexpr bool operator==(const CI <OtherConst>& y, const sentinel & x);
};
}

*/

template<std::ranges::view V>
class take_view : public std::ranges::view_interface<take_view<V>>
{
 private:
  template<bool IsConst>
  class Sentinel {
    private:
      using Base = _intern::maybe_const_t<IsConst, V>;  // exposition only
      template<bool OtherConst>
      using CI = std::counted_iterator<std::ranges::iterator_t<_intern::maybe_const_t<OtherConst, V>>>;  // exposition only
      std::ranges::sentinel_t<Base> end_ = std::ranges::sentinel_t<Base >();  // exposition only
    public:
      Sentinel() = default;
      constexpr explicit Sentinel(std::ranges::sentinel_t<Base> end)
       : end_(end) {
      }
      constexpr Sentinel(Sentinel<!IsConst> s)
        requires IsConst && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base >>
       : end_{std::move(s.end_)} {
      }

      constexpr std::ranges::sentinel_t<Base> base() const;

      friend constexpr bool operator==(const CI <IsConst>& y, const Sentinel & x) {
        return y.count() == 0 || y.base() == x.end_;
      }
      template<bool OtherConst = !IsConst>
      requires std::sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<_intern::maybe_const_t<OtherConst, V>>>
      friend constexpr bool operator==(const CI <OtherConst>& y, const Sentinel & x) {
        return y.count() == 0 || y.base() == x.end_;
      }

      //friend sentinel<!IsConst>;
  };

 private:
  V base_ = V(); // exposition only
  std::ranges::range_difference_t<V> count_ = 0; // exposition only
  // 26.7.10.3, class template take_view::sentinel
  //above: template<bool> class Sentinel ; // exposition only

 public:
  take_view() requires std::default_initializable<V> = default;

  constexpr take_view(V b, std::ranges::range_difference_t<V> c)
   : base_(std::move(b)), count_{c} {
      assert(c >= 0);
  }

  constexpr V base() const & requires std::copy_constructible<V> { return base_; }
  constexpr V base() && { return std::move(base_); }

  // begin() for non-simple views (already in the standard):
  constexpr auto begin() requires (!_intern::simple_view<V>) {
    //std::cout << "take_view::begin() for non-simple views\n";
    if constexpr (std::ranges::sized_range<V>) {
      if constexpr (std::ranges::random_access_range<V>) {
        return std::ranges::begin(base_);
      }
      else {
        auto sz = std::ranges::range_difference_t<V>(size());
        return std::counted_iterator(std::ranges::begin(base_), sz);
      }
    }
    else {
      return std::counted_iterator(std::ranges::begin(base_), count_);
    }
  }
  // begin() const for const-iterable views (already in the standard)
  // is splitted:
  // - the non-const version returns non-const iterators:
  constexpr auto begin() requires std::ranges::range<const V> {
    //std::cout << "take_view::begin() for simple views\n";
    if constexpr (std::ranges::sized_range<const V>) {
      if constexpr (std::ranges::random_access_range<const V>) {
        return std::ranges::begin(base_);
      } 
      else {
        auto sz = std::ranges::range_difference_t<const V>(size());
        return std::counted_iterator(std::ranges::begin(base_), sz);
      }
    } 
    else {
      return std::counted_iterator(std::ranges::begin(base_), count_);
    }
  }
  // - the const version returns const iterators:
  constexpr auto begin() const requires std::ranges::range<const V> {
    //std::cout << "take_view::begin() const for simple views\n";
    if constexpr (std::ranges::sized_range<const V>) {
      if constexpr (std::ranges::random_access_range<const V>) {
        //return std::ranges::begin(base_);
        return make_const_iterator(std::ranges::begin(base_));
      }
      else {
        auto sz = std::ranges::range_difference_t<const V>(size());
        //return std::counted_iterator(std::ranges::begin(base_), sz);
        return make_const_iterator(std::counted_iterator(std::ranges::begin(base_), sz));
      }
    } 
    else {
      //return std::counted_iterator(std::ranges::begin(base_), count_);
      return make_const_iterator(std::counted_iterator(std::ranges::begin(base_), count_));
    }
  }

  // end(): no change (yet)
  // - TODO: how to deal with sentinels in the const case?
  constexpr auto end() requires (!_intern::simple_view<V>) {
    if constexpr (std::ranges::sized_range<V>) {
      if constexpr (std::ranges::random_access_range<V>) {
        //return std::ranges::begin(base_) + std::ranges::range_difference_t<V>(size());
        return make_const_sentinel(std::ranges::begin(base_) + std::ranges::range_difference_t<V>(size()));
      }
      else {
        return std::default_sentinel;
      }
    }
    else {
      return Sentinel<false>{std::ranges::end(base_)};
    }
  }
  constexpr auto end() const requires std::ranges::range<const V> {
    if constexpr (std::ranges::sized_range<const V>) {
      if constexpr (std::ranges::random_access_range<const V>) {
        //return std::ranges::begin(base_) + std::ranges::range_difference_t<const V>(size());
        return make_const_sentinel(std::ranges::begin(base_) + std::ranges::range_difference_t<const V>(size()));
      }
      else {
        return std::default_sentinel;
      }
    }
    else {
      return Sentinel<true>{std::ranges::end(base_)};
    }
  }

  constexpr auto size() requires std::ranges::sized_range<V> {
    auto n = std::ranges::size(base_);
    return std::ranges::min(n, static_cast<decltype(n)>(count_));
  }
  constexpr auto size() const requires std::ranges::sized_range<const V> {
    auto n = std::ranges::size(base_);
    return std::ranges::min(n, static_cast<decltype(n)>(count_));
  }
};

template<typename R>
take_view(R&&, std::ranges::range_difference_t<R>) -> take_view<std::views::all_t<R>>;

} // namespace belleviews

// borrowed if underlying range is borrows (as with std take_view):
template<typename V>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::take_view<V>> = std::ranges::enable_borrowed_range<V>;


//*************************************************************
// belleviews::take()
// bel::views::take()
// 
// A C++ take_view adaptor for the belleviews::take_view
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg, typename DiffT>
  concept can_take_view = requires { take_view(std::declval<Rg>(), std::declval<DiffT>()); };
}

struct _Take {
   // for:  belleviews::take_view{rg, 2}
   template<std::ranges::viewable_range Rg, typename DiffT = std::ranges::range_difference_t<Rg>>
   requires _intern::can_take_view<Rg, DiffT>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg, DiffT diff) const {
     return take_view{std::forward<Rg>(rg), diff};
   }

   // for:  rg | belleviews::take(2)
   template<typename T>
   struct PartialTake {
     T diff;
   };

   template<typename DiffT>
   constexpr auto
   operator() [[nodiscard]] (DiffT diff) const {
     return PartialTake<DiffT>{diff};
   }

   template<typename Rg, typename DiffT>
   friend constexpr auto
   operator| (Rg&& rg, PartialTake<DiffT> pd) {
     return take_view{std::forward<Rg>(rg), pd.diff};
   }
};

inline constexpr _Take take;

} // namespace belleviews

namespace bel::views {
  inline constexpr belleviews::_Take take;
}

#endif // BELLETAKE_HPP
