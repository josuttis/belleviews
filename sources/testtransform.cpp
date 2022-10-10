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

/*
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

*/

template<typename T, typename T2>
concept SupportsAssign = requires (T x, T2 y) { x = y; };


#include "belletransform.hpp"


int main()
{
  std::list coll{1, 2, 3, 4, 5, 6, 7, 8};
  print(coll);

  // **** test functionality:
  auto square = [] (auto val) { return val * val; };

  auto v1 = belleviews::transform_view{coll, square};
  print(v1);

  auto v2 = bel::views::transform(coll, square);
  print(v2);

  //print(coll | std::views::drop(2));  // ERROR withg standard views
  auto v3 = coll | bel::views::transform(square);
  print(v3);                            // OK with belle views

  auto v4 = coll | bel::views::drop(2) | std::views::transform(square);
  print(v4);

  auto v5 = coll | std::views::transform(square) | bel::views::drop(2);
  print(v5);

  // TODO: why doesn't this work?:
  //auto v6 = coll | std::views::transform(square) | bel::views::take(5) | std::views::transform(square);
  //print(v6);

  // **** compare with std::views::drop():
  auto v1std = std::ranges::transform_view{coll, square};
  print(v1std);                           // compile-time ERROR

  auto v3std = coll | std::views::transform(square);
  print(v3std);                           // compile-time ERROR

  auto sumUB = printAndAccum(v3std);        // runtime ERROR (undefined behavior)
  std::cout << "sumUB: " << sumUB << '\n';
  auto sumOK = printAndAccum(v3);           // OK
  std::cout << "sumOK: " << sumOK << '\n';

  // **** test const propagation:
  std::array arr{1, 2, 3, 4, 5, 6, 7, 8};
  print(arr);

  const auto& arr_std_cref = arr | std::views::drop(2);
  *arr_std_cref.begin() += 100;           // OOPS: compiles 
  // with array<const int, 8>:
  //   error: no match for 'operator+=' (operand types are 'const int' and 'int')
  print(arr);

  const auto& arr_bel_cref = arr | bel::views::take(6);
  // *arr_bel_cref.begin() += 100;   // ERROR (good)
                                      //  no match for 'operator+=' (operand types are 'const std::complex<double>' and 'int')
  assert(std::is_const_v<std::remove_reference_t<decltype(*arr_bel_cref.begin())>>);
  print(arr);

  /*
  // test const propagation:
  // - usually we can modify elements:
  auto&& tr0 =  arr | bel::views::transform(square);
  tr0[0] = 42;     // OK
  static_assert(SupportsAssign<decltype(tr0[0]), int>);
  // - but not if view is const:
  const auto& trConst =  arr | bel::views::trasform(square);
  //trConst[0] = 42;     // ERROR
  if constexpr (SupportsAssign<decltype(trConst[0]), int>) {     // ERROR
    std::cerr << "TEST FAILED: can assign so const not propagated\n";
  }
  else {
    std::cerr << "OK: can't assign, so const is propagated\n";
  }
  static_assert(!SupportsAssign<decltype(trConst[0]), int>);
  // - NOTE: a non-const copy of the view can modify elements again: 
  auto tr2 = trConst;
  tr2[0] = 42;     // !!!
  static_assert(SupportsAssign<decltype(tr2[0]), int>);

  // const views should be common range if possible:
  static_assert(std::ranges::common_range<decltype(arr)>);
  static_assert(std::ranges::common_range<decltype(dr0)>);
  //std::cout << typeid(drConst.begin()).name() << '\n';
  //std::cout << typeid(drConst.end()).name() << '\n';
  static_assert(std::ranges::common_range<decltype(drConst)>);
  std::list lst{1, 2, 3, 4, 5, 6, 7, 8};
  const auto& drLst =  lst | bel::views::take(6);
  static_assert(!std::ranges::common_range<decltype(drLst)>);

  // bel drop view IS a borrowed range if the underlying range is (same as in std drop view):
  auto getVec = [] {
    return std::vector<std::string>{"one", "two", "three"};
  };
  auto vec = getVec();
  auto vVecStd = vec | std::views::drop(2);
  static_assert(std::ranges::borrowed_range<decltype(vVecStd)>);
  auto vVecBel = vec | bel::views::drop(2);
  static_assert(std::ranges::borrowed_range<decltype(vVecBel)>);
  auto vTmpVecStd = getVec() | std::views::drop(2);
  static_assert(!std::ranges::borrowed_range<decltype(vTmpVecStd)>);
  auto vTmpVecBel = getVec() | bel::views::drop(2);
  static_assert(!std::ranges::borrowed_range<decltype(vTmpVecBel)>);

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
  */

    /*
  std::cout << "writing transformations:\n";
  std::list<int> lst{1, 2, 3, 4, 5, 6, 7, 8};
  print(lst);
  auto add100 = [] (int& elem) -> int& {
    //std::cout << "process " << elem << '\n';
    return elem += 100;
  };

  // std behavior: const has no effect:
  for (auto& val : lst | bel::views::transform(add100) | bel::views::take(2) | bel::views::transform(add100)) {
  }
  print(lst);

  // std behavior: const has no effect:
  const auto cv = lst | bel::views::transform(add100) | std::views::take(2) | bel::views::transform(add100);
  for (auto& val : cv) {

  }
  print(lst);
  */

  // initialize collection with pairs of int as elements:
  std::vector<std::pair<int,int>> vecPairs{{1,0}, {5,3}, {2,2}, {4,1}, {2,7}};
  auto printPairs = [] (const auto& coll) {
    for (auto& elem : coll) {
      std::cout << elem.first << '/' << elem.second << ' ';
    }
    std::cout << '\n';
  };
  printPairs(vecPairs);

  // increment the smaller of the two values in each pair:
  auto lessOf1st2nd = [] (std::pair<int,int>& elem) -> int& {
    return elem.first > elem.second ? elem.second : elem.first;
  };
  for (auto&& member : vecPairs | bel::views::transform(lessOf1st2nd)) {
    ++member;
  }
  printPairs(vecPairs);

  // std behavior: const has no effect:
  const auto cvStd = vecPairs | std::views::transform(lessOf1st2nd);
  for (auto&& member : cvStd) {
    ++member;
  }
  printPairs(vecPairs);

  // TODO: SHOULD NOT COMPILE: bel behavior: const makes elements const:
  const auto cvBel = vecPairs | bel::views::transform(lessOf1st2nd);
  for (auto&& member : cvBel) {
    ++member;
  }
  printPairs(vecPairs);
}

