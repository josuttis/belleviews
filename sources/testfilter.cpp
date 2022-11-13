#include <iostream>
#include <string>
//#include "string.hpp"   // special string to find use of freed memory
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
  sum = std::reduce(std::execution::par,
                    coll.begin(), coll.end(),
                    sum);
  t1.join();
  return sum;
}

void testFilterCache(const std::string& msg, auto&& coll, auto&& vColl)
{
  std::cout << "\n==== testFilterCache() with " << msg << ":\n";

  // insert a new element at the front (=> 1 2 3 4 5)  
  printUniversal("coll: ",  coll);       // OK:  1 2 3 4 5
  printUniversal("view:     ",  vColl);  // OK:      3 4 5

  // insert more elements at the front (=> 0 1 2 3 4 5)  
  coll.insert(coll.begin(), {98, 99, 0, -1});
  printUniversal("coll: ",  coll);       // OK:  0 1 2 3 4 5
  printUniversal("view:     ",  vColl);  // OK:      3 4 5

  // creating a copy heals:
  auto vColl2 = vColl;

  printUniversal("coll: ",  coll);        // OK:  0 1 2 3 4 5
  printUniversal("copy:     ",  vColl2);  // OK:      2 3 4 5
}

template<typename T, typename T2>
concept SupportsAssign = requires (T x, T2 y) { x = y; };


void testBasics()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  printConst(coll);

  //auto v1 = std::ranges::filter_view{coll, notTimes3};
  auto v1 = belleviews::filter_view{coll, notTimes3};
  printUniversal(v1);
  printConst(v1);

  auto v2 = bel::views::filter(coll, notTimes3);
  printConst(v2);

  auto v3 = coll | bel::views::filter(notTimes3);
  printConst(v3);

  // test wioth other belle views:
  auto v4 = coll | bel::views::drop(2) | bel::views::filter(notTimes3);
  printConst(v4);

  auto v5 = coll | bel::views::take(6) | bel::views::filter(notTimes3) | bel::views::take(2);
  printConst(v5);

  // test with standard views:
  auto v6 = coll | bel::views::drop(2) | std::views::filter(notTimes3);
  //printConst(v6);   // ERROR (standard views disables const printing) 
  printUniversal(v6);

  auto v7 = coll | std::views::filter(notTimes3) | bel::views::drop(2);
  //printConst(v7);  // ERROR (standard views disables const printing) 
  printUniversal(v7);  // ERROR


  // test with common and non-common range:
  {
    std::cout << "--- test filter_view on common range:\n"; 
    auto vCommon = std::views::iota(1, 10);
    auto vfCommon = vCommon | bel::views::filter(notTimes3);
    printConst(vCommon);
    printConst(vfCommon);
    static_assert(std::ranges::common_range<decltype(vCommon)>);
    static_assert(std::ranges::common_range<decltype(vfCommon)>);
  }
  {
    std::cout << "--- test filter_view on non-common range:\n"; 
    auto vNonCommon = std::views::iota(1, 10L);
    auto vfNonCommon = vNonCommon | bel::views::filter(notTimes3);
    printConst(vNonCommon);
    printConst(vfNonCommon);
    static_assert(!std::ranges::common_range<decltype(vNonCommon)>);
    static_assert(!std::ranges::common_range<decltype(vfNonCommon)>);
  }

  // filter view is not a borrowed range:
  auto getVec = [] {
    return std::vector<std::string>{"one", "two", "three"};
  };
  auto vec = getVec();
  auto vVecStd = vec | std::views::filter([](const auto& s){return s[0] == 't';});
  static_assert(!std::ranges::borrowed_range<decltype(vVecStd)>);
  auto vVecBel = vec | bel::views::filter([](const auto& s){return s[0] == 't';});
  static_assert(!std::ranges::borrowed_range<decltype(vVecBel)>);

  // category:
  {
    std::vector vec{1, 2, 3, 4};
    static_assert(std::ranges::random_access_range<decltype(vec)>);
    auto v = vec | std::views::filter(times3);
    static_assert(std::ranges::bidirectional_range<decltype(v)>);
    static_assert(!std::ranges::random_access_range<decltype(v)>);
  }
}


void testConstPropagation()
{
  //**** test const propagation:
  std::array arr{1, 2, 3, 4, 5, 6, 7, 8};
  printConst(arr);

  // test const propagation:
  // - usually we can modify elements:
  auto&& tr0 =  arr | bel::views::filter(notTimes3);
  tr0.front() = 42;  // OK
  // - but not if view is const:
  const auto& tr1 =  arr | bel::views::filter(notTimes3);
  //tr1.front() = 42;     // ERROR
  if constexpr (SupportsAssign<decltype(tr1.front()), int>) {     // ERROR
    std::cerr << "TEST FAILED: can assign so const not propagated\n";
  }
  else {
    std::cerr << "OK: can't assign, so const is propagated\n";
  }
  // - NOTE: a non-const copy of the view can modify elements again: 
  auto tr2 = tr1;
  tr2.front() = 42;     // !!!
}


void testCaching()
{
  //**** caching works as expected: 
  {
  auto biggerThan2 = [](auto v) {
    return v > 2;
  };
  {
    std::vector vec{1, 2, 3, 4, 5};
    testFilterCache("std::views::filter(>2) on vector", vec, vec | std::views::filter(biggerThan2));
  }
  {
    std::list lst{1, 2, 3, 4, 5};
    testFilterCache("std::views::filter(>2) on list", lst, lst | std::views::filter(biggerThan2));
  }
  {
    std::vector vec{1, 2, 3, 4, 5};
    testFilterCache("bel::views::filter(>2) on vector", vec, vec | bel::views::filter(biggerThan2));
  }
  {
    std::list lst{1, 2, 3, 4, 5};
    testFilterCache("bel::views::filter(>2) on list", lst, lst | bel::views::filter(biggerThan2));
  }
  }
  std::cout << '\n';
}


void testConcurrentIteration()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};           // no random-access range
  static_assert(!std::ranges::random_access_range<decltype(coll)>);

  // test concurrent read iterations:
  auto v3Std = coll | std::views::filter(notTimes3);
  //auto sumUB = concurrentPrintAndAccum(v3std);             // RUNTIME ERROR with std views

  auto v3 = coll | bel::views::filter(notTimes3);
  auto sumOK = concurrentPrintAndAccum(v3);                  // OK with belle views
  std::cout << "sumOK: " << sumOK << '\n';
  sumOK = concurrentPrintAndAccum(v3 | std::views::common);  // OK with belle views
  //std::cout << "sumOK: " << sumOK << '\n';
}


void testReadme()
{
  // example in README.md:
  {
    std::vector vec{1, 2, 3, 4, 5};
    printConst(vec);

    auto biggerThan2 = [](auto v) {
      return v > 2;
    };

    auto big2Std = vec | std::views::filter(biggerThan2);
    printUniversal("", big2Std);       // OK:  3 4 5
    auto big2Bel = vec | bel::views::filter(biggerThan2);
    //TODO: doesn't compile with VC++:
    printConst(big2Bel);                    // OK:  3 4 5

    vec.insert(vec.begin(), {9, 0, -1});
    printConst(vec);                        // vec now: 9 0 -1 1 2 3 4 5

    printUniversal("", big2Std);       // OOPS:  -1 3 4 5
    //TODO: doesn't compile with VC++:
    printConst(big2Bel);                    // OK:  9 3 4 5
  }
}


int main()
{
  testBasics();
  testConstPropagation();
  testCaching();
  testConcurrentIteration();
  testReadme();
}

