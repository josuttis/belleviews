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
  auto v1 = belleviews::sub_view{++coll.begin(), --coll.end()};
  print(v1);

  auto v2 = bel::views::sub(++coll.begin(), --coll.end());
  print(v2);

  // **** test const propagation:
  {
    std::cout << "test const propagation:\n";
    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8};
    print(vec);
    auto v = bel::views::sub(++++vec.begin(), --vec.end());
    for (auto&& elem : v) {
      static_assert(!std::is_const_v<std::remove_reference_t<decltype(elem)>>);
      elem = elem * elem;
    }
    print(vec);
    const auto vc = bel::views::sub(++++vec.begin(), --vec.end());
    for (auto&& elem : vc) {
      static_assert(std::is_const_v<std::remove_reference_t<decltype(elem)>>);
      //elem = elem * elem;  // ERROR
    }
  }

  /*
  //print(coll | std::views::drop(2));  // ERROR withg standard views
  auto v3 = coll | bel::views::drop(2);
  print(v3);                            // OK with belle views

  auto v4 = coll | bel::views::drop(2) | std::views::take(4);
  print(v4);

  auto v5 = coll | std::views::take(6) | bel::views::drop(2);
  print(v5);

  //auto v6 = coll | std::views::take(6) | bel::views::drop(2) | std::views::take(2);
  //print(v6);
  */

}

