#ifndef BELLEFILTER_HPP
#define BELLEFILTER_HPP

#include "makeconstiterator.hpp"

#include <concepts>
#include <ranges>
#include <cassert>

namespace belleviews {
  namespace intern
  {
    template<typename _Tp>
      concept __boxable = std::copy_constructible<_Tp> && std::is_object_v<_Tp>;

    template<__boxable _Tp>
      struct __box : std::optional<_Tp>
      {
        using std::optional<_Tp>::optional;

        constexpr
        __box()
        noexcept(std::is_nothrow_default_constructible_v<_Tp>)
        requires std::default_initializable<_Tp>
        : std::optional<_Tp>{std::in_place}
        { }

        __box(const __box&) = default;
        __box(__box&&) = default;

        using std::optional<_Tp>::operator=;

        // _GLIBCXX_RESOLVE_LIB_DEFECTS
        // 3477. Simplify constraints for semiregular-box
        // 3572. copyable-box should be fully constexpr
        constexpr __box&
        operator=(const __box& __that)
        noexcept(std::is_nothrow_copy_constructible_v<_Tp>)
        requires (!std::copyable<_Tp>)
        {
          if (this != std::__addressof(__that))
            {
              if ((bool)__that)
                this->emplace(*__that);
              else
                this->reset();
            }
          return *this;
        }

        constexpr __box&
        operator=(__box&& __that)
        noexcept(std::is_nothrow_move_constructible_v<_Tp>)
        requires (!std::movable<_Tp>)
        {
          if (this != std::__addressof(__that))
            {
              if ((bool)__that)
                this->emplace(std::move(*__that));
              else
                this->reset();
            }
          return *this;
        }
      };
  } //namespace intern
} //namespace belleviews {
// TODO: optimize __box as in gcc


namespace belleviews {

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
  [[no_unique_address]] intern::__box<Pred> pred_;
  // 24.7.4.3, class filter_view::iterator
  //above: class iterator;                // exposition only
  // 24.7.4.4, class filter_view::sentinel
  //above: class sentinel;                // exposition only
 public:

}
*/

template<std::ranges::input_range V,
         std::indirect_unary_predicate<std::ranges::iterator_t<V>> Pred>
requires std::ranges::view<V> && std::is_object_v<Pred>
class filter_view : public std::ranges::view_interface<filter_view<V, Pred>>
{
 private:
  struct Sentinel;

  struct Iterator //: __detail::__filter_view_iter_cat<V>
  {
   private:
    /* TODO:
    static constexpr auto _S_iter_concept() {
      if constexpr (std::ranges::bidirectional_range<V>)
        return std::bidirectional_iterator_tag{};
      else if constexpr (std::ranges::forward_range<V>)
        return std::forward_iterator_tag{};
      else
        return std::input_iterator_tag{};
    }
    */

   private:
    friend filter_view;
    using VIterT = std::ranges::iterator_t<V>;
    VIterT current_ = VIterT();  // exposition only
    filter_view* filterViewPtr = nullptr;                         // exposition only

   public:
    //using iterator_concept = see below ;
    //using iterator_category = see below ;
    using value_type = std::ranges::range_value_t<V>;
    using difference_type = std::ranges::range_difference_t<V>;
    Iterator() requires std::default_initializable<VIterT> = default;  // requires not in standard

    constexpr Iterator(filter_view* par, VIterT cur)
     : current_(std::move(cur)), filterViewPtr{par} {
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
      auto __tmp = *this;
      ++*this;
      return __tmp;
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

  struct ConstIterator //: __detail::__filter_view_iter_cat<V>
  {
   private:
    /* TODO:
    static constexpr auto _S_iter_concept() {
      if constexpr (std::ranges::bidirectional_range<V>)
        return std::bidirectional_iterator_tag{};
      else if constexpr (std::ranges::forward_range<V>)
        return std::forward_iterator_tag{};
      else
        return std::input_iterator_tag{};
    }
    */

   private:
    friend filter_view;
    using VIterT = _intern::const_iterator_t<V>;
    VIterT current_ = VIterT();  // exposition only
    const filter_view* filterViewPtr = nullptr;                         // exposition only

   public:
    //using iterator_concept = see below ;
    //using iterator_category = see below ;
    using value_type = std::ranges::range_value_t<V>;
    using difference_type = std::ranges::range_difference_t<V>;
    ConstIterator() requires std::default_initializable<VIterT> = default;  // requires not in standard

    constexpr ConstIterator(const filter_view* par, VIterT cur)
     : current_(std::move(cur)), filterViewPtr{par} {
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
      current_ = std::ranges::find_if(std::move(++current_),
                                      std::ranges::end(filterViewPtr->base_),
                                      std::ref(*filterViewPtr->pred_));
      return *this;
    }
    constexpr void operator++(int) {
      ++*this;
    }
    constexpr ConstIterator operator++(int) requires std::ranges::forward_range<V> {
      auto __tmp = *this;
      ++*this;
      return __tmp;
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
    friend constexpr std::ranges::range_rvalue_reference_t<V> iter_move(const ConstIterator& i)
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
       return i.current == end_;
     }
   public:
    Sentinel() = default;
    constexpr explicit Sentinel(filter_view& filterView)
     : end_{std::ranges::end(filterView.base_)} {
    }
    constexpr std::ranges::sentinel_t<V> base() const {
      return end_;
    }
    friend constexpr bool operator==(const Iterator& x, const Sentinel& y) {
      return y.equal(x);
    }
  };

 private:
  V base_ = V();
  [[no_unique_address]] intern::__box<Pred> pred_;
  //[[no_unique_address]] __detail::_CachedPosition<V> _M_cached_begin;

 public:
  filter_view() requires (std::default_initializable<V> && std::default_initializable<Pred>)
   = default;

  constexpr
  filter_view(V __base, Pred __pred)
   : base_(std::move(__base)), pred_(std::move(__pred)) {
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
    //return make_const_iterator(Iterator{this, std::move(it)});
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
      //return make_const_iterator(Iterator{this, std::ranges::end(base_)});
    else
      return Sentinel{this};
  }
};

template<class R, class Pred>
filter_view(R&&, Pred) -> filter_view<std::views::all_t<R>, Pred>;

} // namespace belleviews

// TAKE OPEN:
//
/*???
template<typename _Tp>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::filter_view<_Tp>> = std::ranges::enable_borrowed_range<_Tp>;
*/

namespace belleviews {

namespace _intern {
  template<typename Rg, typename Pred>
  concept can_filter_view = requires { filter_view(std::declval<Rg>(), std::declval<Pred>()); };
}

struct _Filter {
   // for: belleviews::filter_view{coll, isequal}
   template<std::ranges::viewable_range Rg, typename Pred>
   requires _intern::can_filter_view<Rg, Pred>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg, Pred&& pred) const {
     return filter_view{std::forward<Rg>(rg), std::forward<Pred>(pred)};
   }

   // for: coll | belleviews::filter(isequal)
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
   operator| (Rg&& rg, PartialFilter<Pred> pd) {
     return filter_view{std::forward<Rg>(rg), pd.pred};
   }
};

inline constexpr _Filter filter;

} // namespace belleviews

namespace bel::views {
  inline constexpr belleviews::_Filter filter;
}

#endif // BELLEFILTER_HPP