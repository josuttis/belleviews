#ifndef MAKECONSTITERATOR_HPP
#define MAKECONSTITERATOR_HPP

#include <functional>
#include <concepts>
#include <ranges>
#include <span>
#include <compare>

////////////////////////////////////////////////////////////////////////
// https://brevzin.github.io/c++/2019/09/23/declarative-cpos/

template <typename It> struct iterator_concept_for { };
template <typename It> requires std::contiguous_iterator<It>
struct iterator_concept_for<It> {
    using iterator_concept = std::contiguous_iterator_tag;
};

template <typename It> struct iterator_category_for { };
template <std::forward_iterator It>
struct iterator_category_for<It> {
    using iterator_category = typename std::iterator_traits<It>::iterator_category;
};

template <std::input_iterator It>
using const_ref_for = std::common_reference_t<std::iter_value_t<It> const &&, std::iter_reference_t<It>>;

template <typename It>
concept ConstantIterator = std::input_iterator<It>
                        && std::same_as<const_ref_for<It>, std::iter_reference_t<It>>;

template <typename R>
concept ConstantRange = std::ranges::range<R> && ConstantIterator<std::ranges::iterator_t<R>>;

template <typename T, typename U>
concept NotSameAs = not std::same_as<T, U>;

template <std::input_iterator It>
class basic_const_iterator : public iterator_concept_for<It>
                           , public iterator_category_for<It>
{
    It it;

public:
    using value_type = std::iter_value_t<It>;
    using difference_type = std::iter_difference_t<It>;
    using reference = const_ref_for<It>;

    basic_const_iterator() = default;
    basic_const_iterator(It it) : it(std::move(it)) { }
    template <std::convertible_to<It> U>
    basic_const_iterator(basic_const_iterator<U> c) : it(std::move(c.base())) { }
    basic_const_iterator(std::convertible_to<It> auto&& c) : it(FWD(c)) { }

    auto operator++() -> basic_const_iterator& { ++it; return *this; }
    auto operator++(int) -> basic_const_iterator requires std::forward_iterator<It> { auto cpy = *this; ++*this; return cpy; }        
    void operator++(int) { ++*this; }

    auto operator--() -> basic_const_iterator& requires std::bidirectional_iterator<It> { --it; return *this; }
    auto operator--(int) -> basic_const_iterator requires std::bidirectional_iterator<It> { auto cpy = *this; --*this; return cpy; }        

    auto operator+(difference_type n) const -> basic_const_iterator requires std::random_access_iterator<It> { return const_iterator(it + n); }
    auto operator-(difference_type n) const -> basic_const_iterator requires std::random_access_iterator<It> { return const_iterator(it - n); }
    friend auto operator+(difference_type n, basic_const_iterator const& rhs) -> basic_const_iterator { return rhs + n; }
    auto operator+=(difference_type n) -> basic_const_iterator& requires std::random_access_iterator<It> { it += n; return *this; }
    auto operator-=(difference_type n) -> basic_const_iterator& requires std::random_access_iterator<It> { it -= n; return *this; }        
    auto operator-(basic_const_iterator const& rhs) const -> difference_type requires std::random_access_iterator<It> { return it - rhs.it; }
    auto operator[](difference_type n) const -> reference requires std::random_access_iterator<It> { return it[n]; }

    auto operator*() const -> reference { return *it; }
    auto operator->() const -> value_type const* requires std::contiguous_iterator<It> { return std::to_address(it); }

    template <std::sentinel_for<It> S>
    auto operator==(S const& s) const -> bool {
        return it == s;
    }

    auto operator<=>(basic_const_iterator const& rhs) const requires std::random_access_iterator<It> {
        return it <=> rhs;
    }

    template <NotSameAs<basic_const_iterator> Rhs>
        requires std::random_access_iterator<It>
             and std::totally_ordered_with<It, Rhs>
    auto operator<=>(Rhs const& rhs) const {
        if constexpr (std::three_way_comparable_with<It, Rhs>) {
            return it <=> rhs;
        } else if constexpr (std::sized_sentinel_for<Rhs, It>) {
            return (it - rhs) <=> 0;
        } else {
            if (it < rhs) return std::strong_ordering::less;
            if (rhs < it) return std::strong_ordering::greater;
            return std::strong_ordering::equal;
        }
    }

    template <std::sized_sentinel_for<It> S>
    auto operator-(S const& s) const -> std::iter_difference_t<It> {
        return it - s;
    }

    template <NotSameAs<basic_const_iterator> S>
        requires std::sized_sentinel_for<S, It>
    friend auto operator-(S const& s, basic_const_iterator const& rhs) -> std::iter_difference_t<It> {
        return s - rhs.it;
    }

    auto base() -> It& { return it; }
    auto base() const -> It const& { return it; }
};

template <typename T, std::common_with<T> U>
struct std::common_type<basic_const_iterator<T>, U> {
    using type = basic_const_iterator<std::common_type_t<T, U>>;
};
template <typename T, std::common_with<T> U>
struct std::common_type<U, basic_const_iterator<T>> {
    using type = basic_const_iterator<std::common_type_t<T, U>>;
};
template <typename T, std::common_with<T> U>
struct std::common_type<basic_const_iterator<T>, basic_const_iterator<U>> {
    using type = basic_const_iterator<std::common_type_t<T, U>>;
};

template <std::input_iterator It>
constexpr auto make_const_iterator(It it) -> ConstantIterator auto {
    if constexpr (ConstantIterator<It>) {
        return it;
    } else {
        return basic_const_iterator<It>(it);
    }
}

template <std::input_iterator It>
using const_iterator = decltype(make_const_iterator(std::declval<It>()));

template <typename S>
constexpr auto make_const_sentinel(S s) {
    if constexpr (std::input_iterator<S>) {
        return make_const_iterator(std::move(s));
    } else {
        return s;
    }
}

//////////////////////////

#endif // MAKECONSTITERATOR_HPP

