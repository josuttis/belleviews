// <belleeagerbegin.hpp> -*- C++ -*-
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

#ifndef BELLEEAGERBEGIN_HPP
#define BELLEEAGERBEGIN_HPP

#include "makeconstiterator.hpp"

#include <concepts>
#include <ranges>
#include <cassert>

//*************************************************************
// class belleviews::eager_begin_view
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

template<typename T>
struct IsVectorTrait {
  constexpr static bool value = false; 
};

template<typename T>
struct IsVectorTrait<std::vector<T>>  {
  constexpr static bool value = true; 
};

template<typename T>
concept IsVector = IsVectorTrait<std::remove_cvref_t<T>>::value;

template<typename T>
concept HasBase = requires(T c){c.base();};

template<std::ranges::range T, bool = requires(T c){c.base();}>
//template<std::ranges::range T, bool = HasBase<T>>
struct BaseType {
  using type = BaseType<decltype(std::declval<T>().base())>::type;
};

template<typename T>
struct BaseType<T, false>  {
  using type = T;   // default false case
};


template<std::ranges::view V>
class eager_begin_view : public std::ranges::view_interface<eager_begin_view<V>>
{
 private:
   static auto&& innerBase(auto&& v) {
     if constexpr (requires{v.base();}) {
       return innerBase(v.base());
     }
     else {
       return v;
     }
   }
 private:
   V base_ = V();
   std::ranges::iterator_t<V> beg_ = std::ranges::begin(base_);
   // to detect broken vector begin:
   //using InnerBaseType = std::remove_cvref_t<BaseType<V>::type>;
   //using InnerBaseType = std::remove_cvref_t<decltype(innerBase(std::declval<V>()))>;
   using InnerBaseType = std::remove_cvref_t<decltype(innerBase(std::declval<V>()))>;
   InnerBaseType* innerBasePtr_ = nullptr;
   void* innerData_ = nullptr;
   //std::ranges::iterator_t<InnerBaseType> innerBaseBeg_{};
 public:
  eager_begin_view() requires std::default_initializable<V> = default;

  constexpr eager_begin_view(V v)
   : base_(std::move(v)), beg_{std::ranges::begin(base_)} {
    if constexpr (IsVector<InnerBaseType>) {
      innerBasePtr_ = &innerBase(v);
      innerData_ = innerBasePtr_->data();
    }
  }

  constexpr V base() const& requires std::copy_constructible<V> { return base_; }
  constexpr V base() && { return std::move(base_); }

  constexpr auto begin()
  {
    if constexpr (IsVector<InnerBaseType>) {
      auto newInnerBasePtr = &innerBase(base_);
      //std::cout << "\ntypeid:   " << typeid(innerBase(base_)).name() << '\n';
      //std::cout << "\ntypeid:   " << typeid(typename BaseType<V>::type).name() << '\n';
      //std::cout << "\nisvector: " << IsVector<typename BaseType<V>::type> << '\n';
      if (newInnerBasePtr == innerBasePtr_ && newInnerBasePtr->data() != innerData_) {
        std::cout << "\n*** BELLEVIEWS RUNTIME ERROR: " << "eager begin() no longer valid: ";
        std::cout << "     " << (void*)(innerBasePtr_->data()) << " <=> " << innerData_ << '\n';
        // as a workaround: copy to itself:
        auto baseCopy = base_;
        base_ = std::move(baseCopy);
        beg_ = std::ranges::begin(base_);
        innerBasePtr_ = &innerBase(base_);
        innerData_ = innerBasePtr_->data();
        return std::ranges::begin(*newInnerBasePtr);
      }
    }
    return beg_;
  }
  constexpr auto begin() const
  {
    return std::make_const_iterator(beg_);
  }
  constexpr auto end()
  {
    return std::ranges::end(base_);
  }
  constexpr auto end() const
  {
    return std::make_const_sentinel(std::ranges::end(base_));
  }
};

/*
template<std::ranges::view V>
requires std::ranges::random_access_range<V>
class eager_begin_view<V> : public std::ranges::view_interface<eager_begin_view<V>>
{
 private:
   V base_ = V();
   std::ranges::range_difference_t<V> offset_ = 0;
 public:
  eager_begin_view() requires std::default_initializable<V> = default;

  constexpr eager_begin_view(V v)
   : base_(std::move(v)) {
       auto baseBeg = std::ranges::begin(base_.base());
       offset_ = std::ranges::begin(base_) - baseBeg;
  }

  constexpr V base() const& requires std::copy_constructible<V> { return base_; }
  constexpr V base() && { return std::move(base_); }

  constexpr auto begin()
  {
    return std::ranges::begin(base_.base()) + offset_;
  }
  constexpr auto begin() const
  {
    return std::make_const_iterator(std::ranges::begin(base_.base()) + offset_);
  }
  constexpr auto end()
  {
    return std::ranges::end(base_);
  }
  constexpr auto end() const
  {
    return std::make_const_sentinel(std::ranges::end(base_));
  }
};
*/

template<typename R>
eager_begin_view(R&&) -> eager_begin_view<std::views::all_t<R>>;

} // namespace belleviews

// always borrowed range
template<typename Rg>
inline constexpr bool std::ranges::enable_borrowed_range<belleviews::eager_begin_view<Rg>> = true;


//*************************************************************
// belleviews::eager_begin()
// bel::views::eager_begin()
// 
// A C++ eager_begin_view adaptor for the belleviews::eager_begin_view
//*************************************************************
namespace belleviews {

namespace _intern {
  template<typename Rg>
  concept can_eager_begin_view = requires { eager_begin_view(std::declval<Rg>()); };
}

struct _EagerBegin {
   // for:  bel::views::eager_begin(rg)
   template<std::ranges::viewable_range Rg>
   requires _intern::can_eager_begin_view<Rg>
   constexpr auto
   operator() [[nodiscard]] (Rg&& rg) const {
     return eager_begin_view{std::forward<Rg>(rg)};
   }

   // for:  rg | bel::views::eager_begin()
   struct PartialEagerBegin {
   };

   constexpr auto
   operator() [[nodiscard]] () const {
     return PartialEagerBegin{};
   }

   template<typename Rg>
   friend constexpr auto
   operator| (Rg&& rg, PartialEagerBegin) {
     return eager_begin_view{std::forward<Rg>(rg)};
   }
};

inline constexpr _EagerBegin eager_begin;

} // namespace belleviews

namespace bel::views {
  inline constexpr belleviews::_EagerBegin eager_begin;
}

#endif // BELLEEAGERBEGIN_HPP
