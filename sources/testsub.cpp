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
void printUniversal(auto&& coll) {
  printUniversal("", std::forward<decltype(coll)>(coll));
}

auto concurrentPrintAndAccum(auto&& coll)
{
  // one thread prints:
  std::jthread t1{[&] { printUniversal("", coll); }};
  // this thead compute the sum (parallel call of begin()):
  std::ranges::range_value_t<decltype(coll)> sum{};
  sum = std::reduce(std::execution::par,
                    coll.begin(), coll.end(),
                    sum);
  t1.join();
  return sum;
}


template<typename T, typename T2>
concept SupportsAssign = requires (T x, T2 y) { x = y; };


void testBasics()
{
  //**** test functionality:
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll);

  belleviews::sub_view v1{++coll.begin(), --coll.end()};
  print(v1);

  bel::subrange v2{++coll.begin(), --coll.end()};
  print(v2);

  auto v3 = belleviews::sub_view{++coll.begin(), --coll.end()};
  print(v3);

  auto v4 = bel::views::sub(++coll.begin(), --coll.end());
  print(v4);
}


void testConstPropagation()
{
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
}


int main()
{
  testBasics();
  testConstPropagation();

}

