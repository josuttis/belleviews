#ifndef BELLEALL_HPP
#define BELLEALL_HPP


//*************************************************************
// belleviews::all()
// bel::views::all()
// bel::views::all_t
// 
// A C++ view adaptor for different belleviews views
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg, typename DiffT>
  concept can_ref_view = requires { ref_view(std::declval<Rg>(), std::declval<DiffT>()); };

  template<typename Rg, typename DiffT>
  concept can_owning_view = requires { owning_view(std::declval<Rg>(), std::declval<DiffT>()); };
}

struct _All {
  // for:  bel::views::all(rg)
  template<std::ranges::viewable_range Rg>
  requires std::ranges::view<Rg> || _intern::can_ref_view<Rg> || _intern::can_owning_view<Rg>
  constexpr auto
  operator() [[nodiscard]] (Rg&& rg) const {
    if constexpr (std::ranges::view<std::decay_t<Rg>>) {
      return std::forward<Rg>(rg);
    }
    else if constexpr (_intern::can_ref_view<Rg>) {
      return ref_view{std::forward<Rg>(rg)};
    }
    else {
      return owning_view{std::forward<Rg>(rg)};
    }
  }
};


inline constexpr _All all;

template<std::ranges::viewable_range Rg>
  using all_t = decltype(all(std::declval<Rg>()));

} // namespace belleviews

namespace bel::views {
  inline constexpr belleviews::_All all;

  template<std::ranges::viewable_range Rg>
    using all_t = decltype(all(std::declval<Rg>()));
}

#endif // BELLEALL_HPP
