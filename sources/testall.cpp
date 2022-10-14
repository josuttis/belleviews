#include <iostream>
#include <array>
#include <vector>
#include <list>
#include <numeric>
#include <thread>
#include <execution>
#include "belleviews.hpp"


void print(const auto& coll)
{
  for (const auto& elem : coll) {
    std::cout << elem << ' ';
  }
  std::cout << '\n';
}

void printUniversal(const std::string& msg, auto&& coll)
{
  std::cout << msg;
  for (const auto& elem : coll) {
    std::cout << elem << ' ';
  }
  std::cout << '\n';
}

auto printAndAccum(auto&& coll)
{
  std::jthread t1{[&] {
                    printUniversal("", coll);
                  }};
  std::ranges::range_value_t<decltype(coll)> sum{};
  sum = std::reduce(std::execution::par,
                    coll.begin(), coll.end(),
                    sum);
  t1.join();
  return sum;
}


template<typename T, typename T2>
concept SupportsAssign = requires (T x, T2 y) { x = y; };


int main()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll);

  //**** test functionality:
  auto v1 = bel::views::all(coll);
  print(v1);
  static_assert(std::same_as<decltype(v1), belleviews::ref_view<decltype(coll)>>);

  auto v2 = bel::views::all(std::vector{1, 2, 3, 4, 5});
  print(v2);
  static_assert(std::same_as<decltype(v2), belleviews::owning_view<std::vector<int>>>);

  auto sum1 = printAndAccum(v1);           // OK
  std::cout << "sum1: " << sum1 << '\n';
  auto sum2 = printAndAccum(v2);           // OK
  std::cout << "sum2: " << sum2 << '\n';


  // **** test const propagation:
  std::array arr{1, 2, 3, 4, 5, 6, 7, 8};
  print(arr);

  // of lvalue (using ref_view):
  const auto& arr_std_cref_ref = std::views::all(arr);
  *arr_std_cref_ref.begin() += 100;           // OOPS: compiles 
  static_assert(!std::is_const_v<std::remove_reference_t<decltype(*arr_std_cref_ref.begin())>>);
  print(arr_std_cref_ref);

  const auto& arr_bel_cref_ref = bel::views::all(arr);
  // *arr_bel_cref_ref.begin() += 100;   // ERROR (good)
  static_assert(std::is_const_v<std::remove_reference_t<decltype(*arr_bel_cref_ref.begin())>>);
  print(arr_bel_cref_ref);

  // of rvalue (using owning_view):
  const auto& arr_std_cref_own = std::views::all(std::move(arr));
  //*arr_std_cref_own.begin() += 100;    // ERROR (std owning view DOES propagate const)
  static_assert(std::is_const_v<std::remove_reference_t<decltype(*arr_std_cref_own.begin())>>);
  print(arr_std_cref_own);

  const auto& arr_bel_cref_own = bel::views::all(std::move(arr));
  // *arr_bel_cref_own.begin() += 100;   // ERROR (good)
  static_assert(std::is_const_v<std::remove_reference_t<decltype(*arr_bel_cref_own.begin())>>);
  print(arr_bel_cref_own);
  

  // test common:

  // test sentinels:

}

