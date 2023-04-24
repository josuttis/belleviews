#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <numeric>
#include <thread>
#include <complex>
#include <execution>
#include "belleviews.hpp"

auto times3 = [] (auto x) { return x % 3 == 0; };
auto notTimes3 = [] (auto x) { return x % 3 != 0; };


void printConst(const auto& coll)
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
  auto common = std::ranges::common_view{coll};
  sum = std::reduce(std::execution::par,
                    common.begin(), common.end(),
                    sum);
  t1.join();
  return sum;
}


template<typename T, typename T2>
concept SupportsAssign = requires (T x, T2 y) { x = y; };


void testBasics()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  printConst(coll);

  //auto v1 = std::ranges::take_while_view{coll, notTimes3};
  auto v1 = belleviews::take_while_view{coll, notTimes3};
  printUniversal(v1);    // as the C++ standard requires
  printConst(v1);             // as we allow

  auto v2 = bel::views::take_while(coll, notTimes3);
  printConst(v2);

  auto v3 = coll | bel::views::take_while(notTimes3);
  printConst(v3);

  // test with other belle views:
  auto v4 = coll | bel::views::take(3) | bel::views::take_while(notTimes3);
  printConst(v4);
  auto v5 = coll | bel::views::take_while(notTimes3) | bel::views::take(2);
  printConst(v5);

  // test with standard views:
  auto v6 = coll | std::views::take(3) | std::views::take_while(notTimes3);
  //printConst(v6);   // ERROR (standard views disables const printing) 
  printUniversal(v6);

  auto v7 = coll | bel::views::take_while(notTimes3) | std::views::take(2);
  printConst(v7);     // OK
  // test with common and non-common range:
  {
    std::cout << "--- test drop_while_view on common range:\n";
    auto vCommon = std::views::iota(1, 10);
    auto vfCommon = vCommon | bel::views::drop_while(notTimes3);
    printConst(vCommon);
    printConst(vfCommon);
    static_assert(std::ranges::common_range<decltype(vCommon)>);
    static_assert(std::ranges::common_range<decltype(vfCommon)>);
  }
  {
    std::cout << "--- test take_while_view on non-common range:\n"; 
    auto vNonCommon = std::views::iota(1, 10L);
    auto vfNonCommon = vNonCommon | bel::views::take_while(notTimes3);
    printConst(vNonCommon);
    printConst(vfNonCommon);
    static_assert(!std::ranges::common_range<decltype(vNonCommon)>);
    static_assert(!std::ranges::common_range<decltype(vfNonCommon)>);
  }

  // unlike drop_while, take_while view is never a borrowed range:
  auto getVec = [] {
    return std::vector<std::string>{"one", "two", "three"};
  };
  auto vec = getVec();
  auto vVecStd = vec | std::views::take_while([](const auto& s){return s[0] == 't';});
  static_assert(!std::ranges::borrowed_range<decltype(vVecStd)>);
  auto vVecBel = vec | bel::views::take_while([](const auto& s){return s[0] == 't';});
  static_assert(!std::ranges::borrowed_range<decltype(vVecBel)>);
}


void testConstPropagation()
{
  // **** test const propagation:
  std::array arr{1, 2, 3, 4, 5, 6, 7, 8};
  printConst(arr);

  // - usually we can modify elements:
  auto&& tr0 =  arr | bel::views::take_while(notTimes3);
  tr0.front() = 42;  // OK
  // - but not if view is const:
  const auto& tr1 =  arr | bel::views::take_while(notTimes3);
  //tr1.front() = 42;     // ERROR
  if constexpr (SupportsAssign<decltype(tr1.front()), int>) {
    std::cerr << "TEST FAILED: can assign, so const not propagated\n";
  }
  else {
    std::cerr << "OK: can't assign, so const is propagated\n";
  }
  // - NOTE: a non-const copy of the view can modify elements again: 
  auto tr2 = tr1;
  tr2.front() = 42;     // !!!
}



void testConcurrentIteration()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};           // no random-access range
  static_assert(!std::ranges::random_access_range<decltype(coll)>);

  // test concurrent read iterations:
  auto v3Std = coll | std::views::take_while(notTimes3);
  //auto sumUB = concurrentPrintAndAccum(v3std);             // RUNTIME ERROR with std views

  auto v3 = coll | bel::views::take_while(notTimes3);
  auto sumOK = concurrentPrintAndAccum(v3);                  // OK with belle views
  std::cout << "sumOK: " << sumOK << '\n';
  sumOK = concurrentPrintAndAccum(v3 | std::views::common);  // OK with belle views
  std::cout << "sumOK: " << sumOK << '\n';
}



int main()
{
  testBasics();
  testConstPropagation();
  testConcurrentIteration();
}

