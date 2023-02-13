#include <iostream>
#include <array>
#include <vector>
#include <list>
#include <numeric>
#include <thread>
#include <complex>
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
  std::array coll2{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll2);

  const auto& coll2_std_cref = coll2 | std::views::take(6);
  *coll2_std_cref.begin() += 100;     // OOPS: compiles 
  // with array<const complex, 8>:
  
  print(coll2);

  const auto& coll2_bel_cref = coll2 | bel::views::take(6);
  //*coll2_bel_cref.begin() += 100;   // ERROR (good)
                                      //  no match for 'operator+=' (operand types are 'const std::complex<double>' and 'int')
  assert(std::is_const_v<std::remove_reference_t<decltype(*coll2_bel_cref.begin())>>);
  print(coll2);

  
  // test const propagation:
  // - usually we can modify elements:
  auto&& tr0 =  coll2 | bel::views::take(6);
  tr0[0] = 42;     // OK
  static_assert(SupportsAssign<decltype(tr0[0]), int>);
  // - but not if view is const:
  const auto& tr1 =  coll2 | bel::views::take(6);
  //tr1[0] = 42;     // ERROR
  if constexpr (SupportsAssign<decltype(tr1[0]), int>) {     // should not be met
    std::cerr << "TEST FAILED: can assign so const not propagated\n";
  }
  else {
    std::cerr << "OK: can't assign, so const is propagated\n";
  }
  static_assert(!SupportsAssign<decltype(tr1[0]), int>);
  // - NOTE: a non-const copy of the view can modify elements again: 
  auto tr2 = tr1;
  tr2[0] = 42;     // !!!
  static_assert(SupportsAssign<decltype(tr2[0]), int>);

  // const views should be common range if possible:
  static_assert(std::ranges::common_range<decltype(tr1)>);
  std::list lst{1, 2, 3, 4, 5, 6, 7, 8};
  const auto& trLst =  lst | bel::views::take(6);
  static_assert(!std::ranges::common_range<decltype(trLst)>);

  // bel take view IS a borrowed range if the underlying range is (same as in std take view):
  auto getVec = [] {
    return std::vector<std::string>{"one", "two", "three"};
  };
  auto vec = getVec();
  auto vVecStd = vec | std::views::take(6);
  static_assert(std::ranges::borrowed_range<decltype(vVecStd)>);
  auto vVecBel = vec | bel::views::take(6);
  static_assert(std::ranges::borrowed_range<decltype(vVecBel)>);
  auto vTmpVecStd = getVec() | std::views::take(6);
  static_assert(!std::ranges::borrowed_range<decltype(vTmpVecStd)>);
  auto vTmpVecBel = getVec() | bel::views::take(6);
  static_assert(!std::ranges::borrowed_range<decltype(vTmpVecBel)>);
}
