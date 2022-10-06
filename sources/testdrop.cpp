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
  printUniversal("copy:     ",  vLst2);  // OK:      2 3 4 5
}

template<typename T, typename T2>
concept SupportsAssign = requires (T x, T2 y) { x = y; };


int main()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll);

  //**** test functionality:
  auto v1 = belleviews::drop_view{coll, 2};
  print(v1);

  auto v2 = bel::views::drop(coll, 2);
  print(v2);

  //print(coll | std::views::drop(2));  // ERROR withg standard views
  auto v3 = coll | bel::views::drop(2);
  print(v3);                            // OK with belle views

  auto v4 = coll | bel::views::drop(2) | std::views::take(4);
  print(v4);

  auto v5 = coll | std::views::take(6) | bel::views::drop(2);
  print(v5);

  //auto v6 = coll | std::views::take(6) | bel::views::drop(2) | std::views::take(2);
  //print(v6);

  //**** compare with std::views::drop():
  auto v1std = std::ranges::drop_view{coll, 2};
  //print(v1std);                           // compile-time ERROR

  auto v3std = coll | std::views::drop(2);
  //print(v3std);                           // compile-time ERROR

  //auto sumUB = printAndAccum(v3std);        // runtime ERROR (undefined behavior)
  //std::cout << "sumUB: " << sumUB << '\n';
  auto sumOK = printAndAccum(v3);           // OK
  std::cout << "sumOK: " << sumOK << '\n';


  //**** test const propagation:
  std::array coll2{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll2);

  const auto& coll2_std_cref = coll2 | std::views::drop(2);
  *coll2_std_cref.begin() += 100;           // OOPS: compiles 
  // with array<const int, 8>:
  //   error: no match for 'operator+=' (operand types are 'const int' and 'int')
  print(coll2);

  const auto& coll2_bel_cref = coll2 | bel::views::take(6);
  //*coll2_bel_cref.begin() += 100;   // ERROR (good)
                                      //  no match for 'operator+=' (operand types are 'const std::complex<double>' and 'int')
  assert(std::is_const_v<std::remove_reference_t<decltype(*coll2_bel_cref.begin())>>);
  print(coll2);


  //**** caching works as expected: 
  {
    std::vector vec{1, 2, 3, 4, 5};
    testDropCache("std::views::drop() on vector", vec, vec | std::views::drop(2));
  }
  {
    std::list lst{1, 2, 3, 4, 5};
    testDropCache("std::views::drop() on list", lst, lst | std::views::drop(2));
  }
  {
    std::list lst{1, 2, 3, 4, 5};
    testDropCache("bel::views::drop() on list", lst, lst | bel::views::drop(2));
  }


  // test const propagation:
  // - usually we can modify elements:
  auto&& dr0 =  coll2 | bel::views::drop(2);
  dr0[0] = 42;     // OK
  // - but not if view is const:
  const auto& dr1 =  coll2 | bel::views::drop(2);
  //dr1[0] = 42;     // ERROR
  if constexpr (SupportsAssign<decltype(dr1[0]), int>) {     // ERROR
    std::cerr << "TEST FAILED: can assign so const not propagated\n";
  }
  else {
    std::cerr << "OK: can't assign, so const is propagated\n";
  }
  // - NOTE: a non-const copy of the view can modify elements again: 
  auto dr2 = dr1;
  dr2[0] = 42;     // !!!

  // example at README.md:
  {
    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8};
    print(vec);
    const auto& vStd = vec | std::views::drop(2);
    vStd[0] += 42;        // OOPS: modifies 1st element in vec
    print(vec);
    const auto& vBel = vec | bel::views::drop(2);
    //vBel[0] += 42;      // ERROR
    auto vBel2 = vBel;    // NOTE: removes constness
    vBel2[0] += 42;       // OK
    print(vec);
  }

  // from README.md:
  {
    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8};

    auto vStd = vec | std::views::drop(2);
    auto sum1 = std::reduce(std::execution::par,      // RUNTIME ERROR (possible data race)
                            vStd.begin(), vStd.end(),
                            0L);
    auto vBel = vec | bel::views::drop(2);
    auto sum2 = std::reduce(std::execution::par,      // OK
                            vBel.begin(), vBel.end(),
                            0L);
  }
}

