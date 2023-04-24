// <belletakewhile.hpp> -*- C++ -*-
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

#ifndef BELLETAKEWHILE_HPP
#define BELLETAKEWHILE_HPP

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::take_while_view
//
// A C++ view
// with the following benefits compared to C++ standard views
// - Always propagates const
// Because
// - This view yields const iterators when it is const
//*************************************************************
namespace belleviews {

    template<std::ranges::view V, typename Pred>
    requires std::ranges::input_range<V> && std::is_object_v<Pred>
            && std::indirect_unary_predicate<const Pred, std::ranges::iterator_t<V>>
    class take_while_view : public std::ranges::view_interface<take_while_view<V, Pred>>
    {
    private:
        template<bool Const>
        struct Sentinel
        {
        private:
            using Base = _intern::maybe_const_t<Const, V>;
            std::ranges::sentinel_t<Base> M_end = std::ranges::sentinel_t<Base>();
            const Pred* M_pred = nullptr;

        public:
            Sentinel() = default;

            constexpr explicit
            Sentinel(std::ranges::sentinel_t<Base> end, const Pred* pred)
                    : M_end(end), M_pred(pred)
            { }

            constexpr
            Sentinel(Sentinel<!Const> s)
            requires Const && std::convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base>>
                    : M_end(s.M_end), M_pred(s.M_pred)
            { }

            constexpr std::ranges::sentinel_t<Base>
            base() const { return M_end; }

            friend constexpr bool
            operator==(const std::ranges::iterator_t<Base>& x, const Sentinel& y)
            { return y.M_end == x || !std::invoke(*y.M_pred, *x); }

            template<bool OtherConst = !Const,
                    typename Base2 = _intern::maybe_const_t<Const, V>>
            requires std::sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base2>>
            friend constexpr bool
            operator==(const std::ranges::iterator_t<Base2>& x, const Sentinel& y)
            { return y.M_end == x || !std::invoke(*y.M_pred, *x); }

            friend Sentinel<!Const>;
        };

        V base_ = V();
        [[no_unique_address]] _intern::SemiregBox<Pred> pred_;

    public:
        take_while_view() requires (std::default_initializable<V>
        && std::default_initializable<Pred>)
        = default;

        constexpr
        take_while_view(V v, Pred pred)
                : base_(std::move(v)), pred_(std::move(pred))
        { }

        constexpr V base() const& requires std::copy_constructible<V> { return base_; }
        constexpr V base() && { return std::move(base_); }

        constexpr const Pred& pred() const { return *pred_; }

        constexpr auto begin()
        // requires (!_intern::simple_view<V>)
        {
            auto it = std::ranges::begin(base_);
            return it;
        }
        constexpr auto begin() const requires std::ranges::range<const V>
        && std::indirect_unary_predicate<const Pred, std::ranges::iterator_t<const V>> {
            auto it = std::ranges::begin(base_);
            return std::make_const_iterator(it);
        }

        constexpr auto end()
        // requires (!_intern::simple_view<V>)
        {
            return Sentinel<false>(std::ranges::end(base_),
                                   std::addressof(*pred_));
        }
        constexpr auto end() const requires std::ranges::range<const V>
        && std::indirect_unary_predicate<const Pred, std::ranges::iterator_t<const V>> {
            return Sentinel<true>(std::ranges::end(base_),
                                  std::addressof(*pred_));
        }
    };

    template<typename R, typename Pred>
    take_while_view(R&&, Pred) -> take_while_view<std::views::all_t<R>, Pred>;

} // namespace belleviews

//*************************************************************
// belleviews::take()
// bel::views::take()
//
// A C++ take_view adaptor for the belleviews::take_view
//*************************************************************
namespace belleviews {

    namespace _intern {
        template<typename Rg, typename Pred>
        concept can_take_while_view = requires { take_while_view(std::declval<Rg>(), std::declval<Pred>()); };
    }

    struct TakeWhile {
        // for:  bel::views::take(rg, 2)
        template<std::ranges::viewable_range Rg, typename Pred>
        requires _intern::can_take_while_view<Rg, Pred>
        constexpr auto
        operator() [[nodiscard]] (Rg&& rg, Pred&& pred) const {
            return take_while_view{std::forward<Rg>(rg), std::forward<Pred>(pred)};
        }

        // for:  rg | bel::views::take_while(pred)
        template<typename T>
        struct PartialTakeWhile {
            T pred;
        };

        template<typename Pred>
        constexpr auto
        operator() [[nodiscard]] (Pred pred) const {
            return PartialTakeWhile<Pred>{pred};
        }

        template<typename Rg, typename Pred>
        friend constexpr auto
        operator| (Rg&& rg, PartialTakeWhile<Pred> pdw) {
            return take_while_view{std::forward<Rg>(rg), pdw.pred};
        }
    };

// belleviews::take_while() :
    inline constexpr TakeWhile take_while;

} // namespace belleviews


namespace bel::views {
    // bel::views::take_while() :
    inline constexpr belleviews::TakeWhile take_while;
}


#endif // BELLETAKEWHILE_HPP
