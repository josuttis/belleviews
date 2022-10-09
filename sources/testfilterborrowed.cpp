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
#define BEL_BORROWED
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


int main()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll);

  //**** test functionality:
  auto notTimes3 = [] (auto x) {
                  return x % 3 != 0;
                };

  //auto v1 = std::ranges::filter_view{coll, notTimes3};
  auto v1 = belleviews::filter_view{coll, notTimes3};
  print(v1);

  auto v2 = bel::views::filter(coll, notTimes3);
  print(v2);

  auto v3 = coll | bel::views::filter(notTimes3);
  print(v3);

  auto v4 = coll | bel::views::drop(2) | bel::views::filter(notTimes3);  // ERROR: can't pass it to std filter
  print(v4);

  /*
   * TODO:
  auto v5 = coll | bel::views::drop(2) | std::views::filter(notTimes3);  // ERROR: can't pass it to std filter
  print(v5);

  auto v6 = coll | std::views::filter(notTimes3) | bel::views::drop(2);
  print(v6);

  auto v7 = coll | bel::views::take(6) | bel::views::filter(notTimes3) | bel::views::take(2);
  print(v7);
  */

  //auto sumUB = printAndAccum(v3std);        // runtime ERROR (undefined behavior)
  //std::cout << "sumUB: " << sumUB << '\n';
  //auto sumOK = printAndAccum(v3);           // ERROR
  //auto sumOK = printAndAccum(v3 | std::views::common);           // OK
  //std::cout << "sumOK: " << sumOK << '\n';

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

  //**** test const propagation:
  std::array arr{1, 2, 3, 4, 5, 6, 7, 8};
  print(arr);

  // test const propagation:
  // - usually we can modify elements:
  auto tr0 =  arr | bel::views::filter(notTimes3);
  /*
  std::cout << "tr0: " << typeid(decltype(tr0)).name() << '\n';
  auto beg = tr0.begin();
  auto end = tr0.end();
  beg == end;
  std::cout << "sentinel for: " << std::sentinel_for<decltype(beg), decltype(end)> << '\n';
  std::cout << "semireg beg:  " << std::semiregular<decltype(beg)> << '\n';
  std::cout << "semireg pred: " << std::semiregular<decltype(notTimes3)> << '\n';
  std::cout << "semireg v:    " << std::semiregular<decltype(tr0)> << '\n';
  using T = decltype(tr0);
  std::cout << std::copyable<T> << ' ';
  std::cout << std::default_initializable<T> << ' ';
  std::cout << std::movable<T> << ' ';
  std::cout << std::copy_constructible<T> << ' ';
  std::cout << std::swappable<T>  << ' ';
  std::cout << '\n'; //std::assignable_from for any T, T&, const T, and const T& to T&.
  auto beg2 = std::ranges::begin(tr0); //.begin();
  //auto end2 = std::ranges::end(tr0); //.end();
  auto qqq = std::ranges::__cust_access::__member_end<decltype(tr0)>; //.end();
  std::cout << "qqq: " << qqq << '\n';
  auto cp = std::ranges::__cust_access::__decay_copy(tr0.end());
  std::cout << "cp: " << typeid(decltype(cp)).name() << '\n';
  std::cout << "range: " << std::ranges::range<decltype(tr0)> << '\n';
  std::cout << "input range: " << std::ranges::input_range<decltype(tr0)> << '\n';
  std::cout << "forward range: " << std::ranges::forward_range<decltype(tr0)> << '\n';
  std::cout << "sentinel for: " << std::sentinel_for<decltype(beg), decltype(end)> << '\n';
  std::cout << typeid(decltype(tr0.begin())).name() << '\n';
  std::cout << typeid(decltype(tr0.end())).name() << '\n';
  for (auto pos = beg; pos != end; ++pos) {
    std::cout << '"' << *pos << "\" ";
  }
  */
  std::cout << '\n';
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

  // example at README.md:
  {
    std::vector vec{1, 2, 3, 4, 5};
    print(vec);

    auto biggerThan2 = [](auto v) {
      return v > 2;
    };

    auto big2Std = vec | std::views::filter(biggerThan2);
    printUniversal("", big2Std);       // OK:  3 4 5
    auto big2Bel = vec | bel::views::filter(biggerThan2);
    //TODO: doesn't compile with VC++:
    print(big2Bel);                    // OK:  3 4 5

    vec.insert(vec.begin(), {9, 0, -1});
    print(vec);                        // vec now: 9 0 -1 1 2 3 4 5

    printUniversal("", big2Std);       // OOPS:  -1 3 4 5
    //TODO: doesn't compile with VC++:
    print(big2Bel);                    // OK:  9 3 4 5
  }

  std::cout << "sizeof iterator: \n"
            << " - for array:  " << sizeof(arr.begin()) << '\n'
            << " - for filter: " << sizeof(tr0.begin()) << '\n';

  // bel filter view IS a borrowed range (in contrast to std filter view):
  auto getVec = [] {
    return std::vector<std::string>{"one", "two", "three"};
  };
  auto vec = getVec();
  auto vVecStd = vec | std::views::filter([](const auto& s){return s[0] == 't';});
  static_assert(!std::ranges::borrowed_range<decltype(vVecStd)>);
  auto vVecBel = vec | bel::views::filter([](const auto& s){return s[0] == 't';});
  static_assert(std::ranges::borrowed_range<decltype(vVecBel)>);
  auto vVecStdBel = vec | std::views::filter([](const auto& s){return s[0] == 't';})
                        | bel::views::filter([](const auto& s){return s[0] == 't';});
  static_assert(!std::ranges::borrowed_range<decltype(vVecStdBel)>);
  auto vVecBelBel = vec | bel::views::filter([](const auto& s){return s[0] == 't';})
                        | bel::views::filter([](const auto& s){return s[0] == 't';});
  static_assert(std::ranges::borrowed_range<decltype(vVecBelBel)>);
  //auto pos2 = std::views::filter(vec, [](const auto& s){return s[0] == 't';}).begin();  // OOPS
  //auto pos3 = std::views::filter(vec, [](const auto& s){return s[0] == 'o';}).begin();  // overwrite memory to force core dump
  //std::cout << * -- ++pos2 << '\n';                                                     // UB
  auto pos4 = bel::views::filter(vec, [](const auto& s){return s[0] == 't';}).begin();  // OK
  auto pos5 = bel::views::filter(vec, [](const auto& s){return s[0] == 'o';}).begin();  // to overwrite memory
  std::cout << * -- ++pos5 << '\n';                                                     // OK
}

