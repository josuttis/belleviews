#ifndef BELLEDROPWHILE_HPP
#define BELLEDROPWHILE_HPP

#include "makeconstiterator.hpp"

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::drop_while_view
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
//*************************************************************
namespace belleviews {

template<std::ranges::view V, typename Pred>
requires std::ranges::input_range<V> && std::is_object_v<Pred>
          && std::indirect_unary_predicate<const Pred, std::ranges::iterator_t<V>>
class drop_while_view : public std::ranges::view_interface<drop_while_view<V, Pred>>
{
    private:
      V base_ = V();
      [[no_unique_address]] _intern::box<Pred> pred_;

    public:
      drop_while_view() requires (std::default_initializable<V>
                                  && std::default_initializable<Pred>)
        = default;

      constexpr
      drop_while_view(V v, Pred pred)
        : base_(std::move(v)), pred_(std::move(pred))
      { }

      constexpr V base() const& requires std::copy_constructible<V> { return base_; }
      constexpr V base() && { return std::move(base_); }

      constexpr const Pred& pred() const { return *pred_; }

      constexpr auto begin() {
        auto it = std::ranges::find_if_not(std::ranges::begin(base_), std::ranges::end(base_),
                                           std::cref(*pred_));
        return it;
      }
      constexpr auto begin() const {
        auto it = std::ranges::find_if_not(std::ranges::begin(base_), std::ranges::end(base_),
                                           std::cref(*pred_));
        return make_const_iterator(it);
      }

      constexpr auto end() {
        return std::ranges::end(base_);
      }
      constexpr auto end() const {
        return make_const_sentinel(std::ranges::end(base_));
      }
};



template<typename R, typename Pred>
drop_while_view(R&&, Pred) -> drop_while_view<std::views::all_t<R>, Pred>;

} // namespace belleviews

// borrowed if underlying range is borrowed (as with std drop_while_view):
template<typename Rg, typename Pred>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::drop_while_view<Rg, Pred>> = std::ranges::enable_borrowed_range<Rg>;


//*************************************************************
// belleviews::drop()
// bel::views::drop()
// 
// A C++ drop_view adaptor for the belleviews::drop_view
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg, typename Pred>
  concept can_drop_while_view = requires { drop_while_view(std::declval<Rg>(), std::declval<Pred>()); };
}

struct _DropWhile {
   // for:  bel::views::drop(rg, 2)
   template<std::ranges::viewable_range Rg, typename Pred>
   requires _intern::can_drop_while_view<Rg, Pred>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg, Pred&& pred) const {
     return drop_while_view{std::forward<Rg>(rg), std::forward<Pred>(pred)};
   }

   // for:  rg | bel::views::drop_while(pred)
   template<typename T>
   struct PartialDropWhile {
     T pred;
   };

   template<typename Pred>
   constexpr auto
   operator() [[nodiscard]] (Pred pred) const {
     return PartialDropWhile<Pred>{pred};
   }

   template<typename Rg, typename Pred>
   friend constexpr auto
   operator| (Rg&& rg, PartialDropWhile<Pred> pdw) {
     return drop_while_view{std::forward<Rg>(rg), pdw.pred};
   }
};

inline constexpr _DropWhile drop_while;

} // namespace belleviews

namespace bel::views {
  inline constexpr belleviews::_DropWhile drop_while;
}


#endif // BELLEDROPWHILE_HPP
