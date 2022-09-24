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

void testDropCache(const std::string& msg, auto&& lst, auto&& vLst)
{
  std::cout << "\n==== testDropCache() with " << msg << ":\n";

  // insert a new element at the front (=> 1 2 3 4 5)  
  printUniversal("coll: ",  lst);       // OK:  1 2 3 4 5
  printUniversal("view:     ",  vLst);  // OK:      3 4 5

  // insert more elements at the front (=> 0 1 2 3 4 5)  
  lst.insert(lst.begin(), 0);
  printUniversal("coll: ",  lst);       // OK:  0 1 2 3 4 5
  printUniversal("view:     ",  vLst);  // OK:      3 4 5

  // creating a copy heals:
  auto vLst2 = vLst;

  printUniversal("coll: ",  lst);        // OK:  0 1 2 3 4 5
  printUniversal("view:     ",  vLst2);  // OK:      2 3 4 5
}


int main()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll);

  //**** test functionality:
  auto v1 = basicviews::drop_view{coll, 2};
  print(v1);

  auto v2 = basicviews::drop(coll, 2);
  print(v2);

  auto v3 = coll | basicviews::drop(2);
  print(v3);

  auto v4 = coll | basicviews::drop(2) | std::views::take(4);
  print(v4);

  auto v5 = coll | std::views::take(6) | basicviews::drop(2);
  print(v5);

  //auto v6 = coll | std::views::take(6) | basicviews::drop(2) | std::views::take(2);
  //print(v6);

  //**** compare with std::views::drop():
  auto v1std = std::ranges::drop_view{coll, 2};
  //print(v1std);                           // compile-time ERROR

  auto v3std = coll | std::views::drop(2);
  //print(v3std);                           // compile-time ERROR

  auto sumUB = printAndAccum(v3std);        // runtime ERROR (undefined behavior)
  std::cout << "sumUB: " << sumUB << '\n';
  auto sumOK = printAndAccum(v3);           // OK
  std::cout << "sumOK: " << sumOK << '\n';


  //**** test const propagation:
  std::vector coll2{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll2);

  const auto& coll2_std_cref = coll2 | std::views::drop(2);
  *coll2_std_cref.begin() += 100;           // OOPS: compiles 
  print(coll2);

  const auto& coll2_basic_cref = coll2 | basicviews::drop(2);
  //*coll2_basic_cref.begin() += 100;       // ERROR: GOOD
  print(coll2);


  //**** caching works as expected: 
  {
    std::list lst{1, 2, 3, 4, 5};
    testDropCache("std::views::drop()", lst, lst | std::views::drop(2));
  }
  {
    std::list lst{1, 2, 3, 4, 5};
    testDropCache("basicviews::drop()", lst, lst | basicviews::drop(2));
  }
}

