// <bellefilter.hpp> -*- C++ -*-
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

#ifndef BELLEFILTER_HPP
#define BELLEFILTER_HPP

#include "makeconstiterator.hpp"

#include <concepts>
#include <ranges>
#include <cassert>
#include <optional>

//*************************************************************
// class belleviews::filter_view
// 
// A C++ view
// with the following benefits compared to C++ standard views
// - Iterating is stateles
//   - Can iterate over elements when the view is const
//   - Supports concurrent iterations
//   - Read iterations do not affect later statements
// - Always propagates const
// Because
// - This view does never cache begin()
// - This view yields const iterators when it is const
// OPEN/TODO:
// - concept and category
//*************************************************************
namespace belleviews {


  /*
template<std::ranges::input_range V,
         std::indirect_unary_predicate<std::ranges::iterator_t<V>> Pred>
requires std::ranges::view<V> && std::is_object_v<Pred>
class filter_view : public std::ranges::view_interface<filter_view<V, Pred>>
{
 private:
  V base_ = V();                 // exposition only
  //semiregular-box<Pred> pred_;   // exposition only
  // TODO: semiregular box:
  [[no_unique_address]] _intern::box<Pred> pred_;
  // 24.7.4.3, class filter_view::iterator
  //above: class iterator;                // exposition only
  // 24.7.4.4, class filter_view::sentinel
  //above: class sentinel;                // exposition only
 public:

}
*/

namespace _intern
{
  template<typename _Base>
  struct filter_view_iter_cat {
  };

  template<std::ranges::forward_range Base>
  struct filter_view_iter_cat<Base> {
   private:
    static auto _S_iter_cat() {
      using _Cat = typename std::iterator_traits<std::ranges::iterator_t<Base>>::iterator_category;
      if constexpr (std::derived_from<_Cat, std::bidirectional_iterator_tag>)
        return std::bidirectional_iterator_tag{};
      else if constexpr (std::derived_from<_Cat, std::forward_iterator_tag>)
        return std::forward_iterator_tag{};
      else
        return _Cat{};
    }
   public:
    using iterator_category = decltype(_S_iter_cat());
  };
} // namespace _intern

template<std::ranges::input_range V,
         std::indirect_unary_predicate<std::ranges::iterator_t<V>> Pred>
requires std::ranges::view<V> && std::is_object_v<Pred>
class filter_view : public std::ranges::view_interface<filter_view<V, Pred>>
{
 private:
  struct Iterator : _intern::filter_view_iter_cat<V>
  {
   private:
    static constexpr auto _S_iter_concept() {
      if constexpr (std::ranges::bidirectional_range<V>)
        return std::bidirectional_iterator_tag{};
      else if constexpr (std::ranges::forward_range<V>)
        return std::forward_iterator_tag{};
      else
        return std::input_iterator_tag{};
    }

   public:
    using iterator_concept = decltype(_S_iter_concept());
    //using iterator_category = FROM BASE CLASS _intern::filter_view_iter_cat<V>
    using value_type = std::ranges::range_value_t<V>;
    using difference_type = std::ranges::range_difference_t<V>;

   private:
    friend filter_view;
    using VIterT = std::ranges::iterator_t<V>;
    const filter_view* filterViewPtr = nullptr;           // view we iterate over
    VIterT current_ = VIterT();                           // current position
    std::optional<V> base_ = {};                          // view my not be def.constructible
    //[[no_unique_address]] _intern::box<Pred> pred_;     // TODO: better?
    Pred pred_;

   public:
    Iterator() requires std::default_initializable<VIterT> = default;  // requires not in standard
    constexpr Iterator(filter_view* pFv, VIterT cur)
     : filterViewPtr{pFv}, current_(std::move(cur)) {
    }

    constexpr const VIterT& base() const& noexcept {
      return current_;
    }
    constexpr VIterT base() && {
      return std::move(current_);
    }

    constexpr std::ranges::range_reference_t<V> operator*() const {
      return *current_;
    }
    constexpr VIterT operator->() const
        requires _intern::has_arrow<VIterT> && std::copyable<VIterT> {
      return current_;
    }

    constexpr Iterator& operator++() {
      current_ = std::ranges::find_if(std::move(++current_),
                                      std::ranges::end(filterViewPtr->base_),
                                      std::ref(*filterViewPtr->pred_));
      return *this;
    }
    constexpr void operator++(int) {
      ++*this;
    }
    constexpr Iterator operator++(int) requires std::ranges::forward_range<V> {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr Iterator& operator--() requires std::ranges::bidirectional_range<V> {
      do {
        --current_;
      }
      while (!std::invoke(*filterViewPtr->pred_, *current_));
      return *this;
    }
    constexpr Iterator operator--(int) requires std::ranges::bidirectional_range<V> {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    friend constexpr bool operator==(const Iterator& x, const Iterator& y)
      requires std::equality_comparable<VIterT> {
        return x.current_ == y.current_;
    }
    friend constexpr std::ranges::range_rvalue_reference_t<V> iter_move(const Iterator& i)
      noexcept(noexcept(std::ranges::iter_move(i.current_))) {
        return std::ranges::iter_move(i.current_);
    }
    friend constexpr void iter_swap(const Iterator& x, const Iterator& y)
      noexcept(noexcept(std::ranges::iter_swap(x.current_, y.current_)))
      requires std::indirectly_swappable<VIterT> {
        std::ranges::iter_swap(x.current_, y.current_);
    }
  };

  struct ConstIterator : _intern::filter_view_iter_cat<V>
  {
   private:
    static constexpr auto _S_iter_concept() {
      if constexpr (std::ranges::bidirectional_range<V>)
        return std::bidirectional_iterator_tag{};
      else if constexpr (std::ranges::forward_range<V>)
        return std::forward_iterator_tag{};
      else
        return std::input_iterator_tag{};
    }

   public:
    using iterator_concept = decltype(_S_iter_concept());
    //using iterator_category = FROM BASE CLASS _intern::filter_view_iter_cat<V>
    using value_type = std::ranges::range_value_t<V>;
    using difference_type = std::ranges::range_difference_t<V>;

   private:
    friend filter_view;
    using VIterT = _intern::const_iterator_t<V>;
    const filter_view* filterViewPtr = nullptr;           // view we iterate over
    VIterT current_ = VIterT();                           // current position
    std::optional<V> base_ = {};                          // view my not be def.constructible
    //[[no_unique_address]] _intern::box<Pred> pred_;     // TODO: better?
    Pred pred_;

   public:
    ConstIterator() requires std::default_initializable<VIterT> = default;  // requires not in standard
    constexpr ConstIterator(const filter_view* pFv, VIterT cur)
     : filterViewPtr{pFv}, current_(std::move(cur)) {
    }

    constexpr const VIterT& base() const& noexcept {
      return current_;
    }
    constexpr VIterT base() && {
      return std::move(current_);
    }

    constexpr _intern::range_const_reference_t<V> operator*() const {
      return *current_;
    }
    constexpr VIterT operator->() const
        requires _intern::has_arrow<VIterT> && std::copyable<VIterT> {
      return current_;
    }

    constexpr ConstIterator& operator++() {
      auto end = std::ranges::end(filterViewPtr->base_);
      //current_ = std::ranges::find_if(std::move(++current_),
      //                                end,
      //                                std::ref(*filterViewPtr->pred_));
      do {
        ++current_;
      }
      while (current_ != end && !std::invoke(*filterViewPtr->pred_, *current_));
      return *this;
    }
    constexpr void operator++(int) {
      ++*this;
    }
    constexpr ConstIterator operator++(int) requires std::ranges::forward_range<V> {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr ConstIterator& operator--() requires std::ranges::bidirectional_range<V> {
      do {
        --current_;
      }
      while (!std::invoke(*filterViewPtr->pred_, *current_));
      return *this;
    }
    constexpr ConstIterator operator--(int) requires std::ranges::bidirectional_range<V> {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    friend constexpr bool operator==(const ConstIterator& x, const ConstIterator& y)
      requires std::equality_comparable<VIterT> {
        return x.current_ == y.current_;
    }
    friend constexpr _intern::range_const_rvalue_reference_t<V> iter_move(const ConstIterator& i)
      noexcept(noexcept(std::ranges::iter_move(i.current_))) {
        return std::ranges::iter_move(i.current_);
    }
    friend constexpr void iter_swap(const ConstIterator& x, const ConstIterator& y)
      noexcept(noexcept(std::ranges::iter_swap(x.current_, y.current_)))
      requires std::indirectly_swappable<VIterT> {
        std::ranges::iter_swap(x.current_, y.current_);
    }
  };


 private:
  class Sentinel {
   private:
     std::ranges::sentinel_t<V> end_ = std::ranges::sentinel_t<V>(); // exposition only

     constexpr bool equal(const Iterator& i) const {
       return i.current_ == end_;
     }
   public:
    Sentinel() = default;
    constexpr explicit Sentinel(filter_view* filterViewPtr)
     : end_{std::ranges::end(filterViewPtr->base_)} {
    }
    constexpr std::ranges::sentinel_t<V> base() const {
      return end_;
    }
    friend constexpr bool operator==(const Iterator& x, const Sentinel& y) {
      return y.equal(x);
    }
  };
  class ConstSentinel {
   private:
     _intern::const_sentinel_t<V> end_ = _intern::const_sentinel_t<V>(); // exposition only

     constexpr bool equal(const ConstIterator& i) const {
       return i.current_ == end_;
     }
   public:
    ConstSentinel() = default;
    constexpr explicit ConstSentinel(const filter_view* filterViewPtr)
     : end_{std::ranges::end(filterViewPtr->base_)} {
    }
    constexpr _intern::const_sentinel_t<V> base() const {
      return end_;
    }
    friend constexpr bool operator==(const ConstIterator& x, const ConstSentinel& y) {
      return y.equal(x);
    }
  };

 private:
  V base_ = V();
  [[no_unique_address]] _intern::box<Pred> pred_;
  //[[no_unique_address]] __detail::_CachedPosition<V> _M_cached_begin;

 public:
  filter_view() requires (std::default_initializable<V> && std::default_initializable<Pred>)
   = default;

  constexpr
  filter_view(V bs, Pred prd)
   : base_(std::move(bs)), pred_(std::move(prd)) {
  }

  constexpr V base() const& requires std::copy_constructible<V> { return base_; }
  constexpr V base() && { return std::move(base_); }
  constexpr const Pred& pred() const {
    return *pred_;
  }

  constexpr Iterator begin() {
    //std::cout << "filter_view::begin()\n";
    assert(pred_.has_value());
    auto it = std::ranges::find_if(std::ranges::begin(base_),
                                   std::ranges::end(base_),
                                   std::ref(*pred_));
    return Iterator{this, std::move(it)};
  }
  constexpr ConstIterator begin() const {
    //std::cout << "filter_view::begin() const\n";
    assert(pred_.has_value());
    auto it = std::ranges::find_if(std::ranges::begin(base_),
                                   std::ranges::end(base_),
                                   std::ref(*pred_));
    return ConstIterator{this, std::move(it)};
    //return std::make_const_iterator(Iterator{this, std::move(it)});
  }

  constexpr auto end() {
    if constexpr (std::ranges::common_range<V>)
      return Iterator{this, std::ranges::end(base_)};
    else
      return Sentinel{this};
  }
  constexpr auto end() const {
    if constexpr (std::ranges::common_range<V>)
      return ConstIterator{this, std::ranges::end(base_)};
      //return std::make_const_iterator(Iterator{this, std::ranges::end(base_)});
    else
      return ConstSentinel{this};
  }
};

template<class R, class Pred>
filter_view(R&&, Pred) -> filter_view<std::views::all_t<R>, Pred>;

} // namespace belleviews

// NO BORROWED VIEW:
//template<typename V, typename Pred>
//inline constexpr bool std::ranges::enable_borrowed_range<belleviews::filter_view<V, Pred>> = std::ranges::enable_borrowed_range<V>;


//*************************************************************
// belleviews::filter()
// bel::views::filter()
// 
// A C++ filter_view adaptor for the belleviews::filter_view
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg, typename Pred>
  concept can_filter_view = requires { filter_view(std::declval<Rg>(), std::declval<Pred>()); };
}

struct _Filter {
   // for:  bel::views::filter(rg, pred)
   template<std::ranges::viewable_range Rg, typename Pred>
   requires _intern::can_filter_view<Rg, Pred>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg, Pred&& pred) const {
     return filter_view{std::forward<Rg>(rg), std::forward<Pred>(pred)};
   }

   // for:  rg | bel::views::filter(pred)
   template<typename T>
   struct PartialFilter {
     T pred;
   };

   template<typename Pred>
   constexpr auto
   operator() [[nodiscard]] (Pred pred) const {
     return PartialFilter<Pred>{pred};
   }

   template<typename Rg, typename Pred>
   friend constexpr auto
   operator| (Rg&& rg, PartialFilter<Pred> pf) {
     return filter_view{std::forward<Rg>(rg), pf.pred};
   }
};

inline constexpr _Filter filter;

} // namespace belleviews

namespace bel::views {
  inline constexpr belleviews::_Filter filter;
}


/* STD:
  namespace std::ranges {
    template<input_range V, indirect_unary_predicate<iterator_t<V>> Pred>
    requires view<V> && is_object_v<Pred>
    class filter_view : public view_interface<filter_view<V, Pred>> {
     private:
      V base_ = V(); // exposition only
      semiregular-box<Pred> pred_; // exposition only
      // 24.7.4.3, class filter_view::iterator
      class iterator ; // exposition only
      // 24.7.4.4, class filter_view::sentinel
      class sentinel ; // exposition only
     public:
      filter_view() = default;
      constexpr filter_view(V base, Pred pred);
      constexpr V base() const& requires copy_constructible<V> { return base_; }
      constexpr V base() && { return std::move(base_); }
      constexpr const Pred& pred() const;
      constexpr iterator begin();
      constexpr auto end() {
        if constexpr (common_range<V>)
          return iterator {*this, ranges::end(base_)};
        else
          return sentinel {*this};
      }
    };
    template<class R, class Pred>
    filter_view(R&&, Pred) -> filter_view<views::all_t<R>, Pred>;
  }

  namespace std::ranges {
    template<input_range V, indirect_unary_predicate<iterator_t<V>> Pred>
    requires view<V> && is_object_v<Pred>
    class filter_view<V, Pred>::iterator {
     private:
      iterator_t<V> current_ = iterator_t<V>(); // exposition only
      filter_view* parent_ = nullptr; // exposition only
     public:
      using iterator_concept = see below ;
      using iterator_category = see below ;
      using value_type = range_value_t<V>;
      using difference_type = range_difference_t<V>;
      iterator () = default;
      constexpr iterator (filter_view& parent, iterator_t<V> current);
      constexpr iterator_t<V> base() const &
      requires copyable<iterator_t<V>>;
      constexpr iterator_t<V> base() &&;
      constexpr range_reference_t<V> operator*() const;
      constexpr iterator_t<V> operator->() const
      requires has-arrow <iterator_t<V>> && copyable<iterator_t<V>>;
      constexpr iterator & operator++();
      constexpr void operator++(int);
      constexpr iterator operator++(int) requires forward_range<V>;
      constexpr iterator & operator--() requires bidirectional_range<V>;
      constexpr iterator operator--(int) requires bidirectional_range<V>;
      friend constexpr bool operator==(const iterator & x, const iterator & y)
      requires equality_comparable<iterator_t<V>>;
      friend constexpr range_rvalue_reference_t<V> iter_move(const iterator & i)
      noexcept(noexcept(ranges::iter_move(i.current_)));
      friend constexpr void iter_swap(const iterator & x, const iterator & y)
      noexcept(noexcept(ranges::iter_swap(x.current_, y.current_)))
      requires indirectly_swappable<iterator_t<V>>;
    };
  }

  namespace std::ranges {
    template<input_range V, indirect_unary_predicate<iterator_t<V>> Pred>
    requires view<V> && is_object_v<Pred>
    class filter_view<V, Pred>::sentinel {
     private:
      sentinel_t<V> end_ = sentinel_t<V>(); // exposition only
     public:
      sentinel () = default;
      constexpr explicit sentinel (filter_view& parent);
      constexpr sentinel_t<V> base() const;
      friend constexpr bool operator==(const iterator & x, const sentinel & y);
    };
  }
*/

#endif // BELLEFILTER_HPP
