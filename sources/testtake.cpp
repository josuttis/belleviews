#include <iostream>
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


int main()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll);

  //**** test functionality:
  auto v1 = belleviews::take_view{coll, 6};
  print(v1);

  auto v2 = bel::views::take(coll, 6);
  print(v2);

  auto v3 = coll | bel::views::take(6);
  print(v3);

  auto v4 = coll | bel::views::drop(2) | std::views::take(4);
  print(v4);

  auto v5 = coll | std::views::take(6) | bel::views::drop(2);
  print(v5);

  //auto v6 = coll | std::views::take(6) | bel::views::take(2) | std::views::take(2);
  //print(v6);

  //auto sumUB = printAndAccum(v3std);        // runtime ERROR (undefined behavior)
  //std::cout << "sumUB: " << sumUB << '\n';
  //auto sumOK = printAndAccum(v3);           // ERROR
  auto sumOK = printAndAccum(v3 | std::views::common);           // OK
  std::cout << "sumOK: " << sumOK << '\n';


  //**** test const propagation:
  std::vector coll2{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll2);

  const auto& coll2_std_cref = coll2 | std::views::drop(2);
  *coll2_std_cref.begin() += 100;           // OOPS: compiles 
  print(coll2);

  const auto& coll2_bel_cref = coll2 | bel::views::drop(2);
  //*coll2_bel_cref.begin() += 100;       // ERROR: GOOD
  print(coll2);
}

