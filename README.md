# Belle Views for C++

Belleviews - A library of C++ views that just works for all basic use cases as expected.

Belleviews are easier to use, more robust,
and cause less unexpected compile-time and runtime errors
than the views of the C++ standard library.

## Motivation

C++20 introduced the ranges library which contain views.
Views are lightweight ranges that refer to (a subset of) a range
with optional transformation of the values.

Views are simple and great to use and the standard ranges library and its implementation
are a tremendous piece of work.

However, for the standard views, some design decisions were made
that are confusing and error-prone and cause sever problems
because these decisions break a couple of basic idioms that collections
(such as standard containers) usually provide.

In fact, **C++ standard views have the following problems:**

- You might **not be able to iterate** over the elements of a standard view **when the view is const**.

  As a consequence:
  - Generic code for both containers and views has to declare the parameters as universal/forwarding references.

- **Concurrent iterations** might cause **undefined behavior** (due to a data race).

  This kind of fatal runtime error is not obvious to see.

- **Read iterations** might **affect** the functional behavior
   of later iterations.

- Standard views do **not propagate constness**.
   This means that declaring the view const does not declare the elements to be const.

- For some standard views **declaring the elements as const** does have **no effect**.

   You might be able to modify the members of a const element of a view.

- **Copying** a view might create a view that has a **different state** and behavior than the source view.

- **`cbegin()`** and `cend()` might **not** make elements **`const`**.

- Type **const_iterator** is **not available**.

- A standard view is not always a pure subset restricting or dealing with the elements of a range,
   but not providing options the range would not provide.
   They might remove constraints and provide options, the range itself does not have. 

This makes the use of views error-prone, confusing and programmers might easily create fatal runtime errors
without being aware of it.

There are reasons for this behavior:
- The key problem is the decision that views might cache begin() so that
  iterating over elements is not stateless.
  The funny consequence is that due to the several consequences you should use standard views ad hoc,
  so that the optimization (which only helps for a second iteration) often doesn't pay off.
- The designers of the views considered views as pointers instead of
  references to collections. However view are no pointers (they do not provide operator * or ->).
  They only internally use pointers.
  The result is a huge confusion of basic rules like const propagation,
  which usually is expected to apply to collection types.
  (Don't get me wrong; you could and can always implement collection types that
   do not propagate constness; however, not everythign you can do as an expert is usefull for the mass.)

However, for the mass, these are too many compromises for not enough value.
For the mass, **we can do better**.

Unfortunately, it is too late to fix the C++ standard views anymore.
For this reason, as an alternative for the standard views, this library is implemented.

The belleviews library provides views that are simple to use and reduce the number of suprises causing compiletime errpors and runtime errors.
The key benefit is that with these views,
programs are way more predictable in behavior and meet the expectations of ordinary programmers.
This especially helps in generic code that may be used for both containers and views.

In some cases the price may be a potentially worse performance.
However, the library will provide workarounds (that help locally istead of comprimising basic collection idioms).

This library does not replace the ranges library.
It only is an alternative for the views in the standard library.
Usually, it should even be possible to combione both.

## How to use Belle Views

So far, we have only a couple of header files, you have to include and use.

In your code, all you have to do usually is to use the namespace `bel::views` instead of `std::views`.
However, in some cases you might have or want to do more:
- Use `bel::views::sub(beg,end)` instead of `std::ranges::subrange{beg,end}`

## Key Examples

Belle views can always iterate over elements even if the views are `const`:

    template<typename T>
    void print(const T& coll);           // print elements of a collection
    
    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8};
    std::list lst{1, 2, 3, 4, 5, 6, 7, 8};

    print(vec | std::views::drop(2));    // OK
    print(lst | std::views::drop(2));    // ERROR with standard views
    print(vec | bel::views::drop(2));    // OK
    print(lst | bel::views::drop(2));    // OK with belle views

Belle views propagate `const`:

    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8};

    const auto& vStd = vec | std::views::drop(2);
    vStd[0] += 42;        // OOPS: modifies 1st element in vec

    const auto& vBel = vec | bel::views::drop(2);
    vBel[0] += 42;        // ERROR (good!)

    auto vBel2 = vBel;    // NOTE: removes constness
    vBel2[0] += 42;       // OK

Belle views do not cache so that they operate as expected way later than defined:

    std::vector vec{1, 2, 3, 4, 5};
    auto biggerThan2 = [](auto v) { return v > 2; };

    auto big2Std = vec | std::views::filter(biggerThan2);
    printUniversal(big2Std);           // OK:  3 4 5
    auto big2Bel = vec | bel::views::filter(biggerThan2);
    print(big2Bel);                    // OK:  3 4 5
    
    vec.insert(vec.begin(), {9, 0, -1});
    print(vec);                        // vec now: 9 0 -1 1 2 3 4 5

    printUniversal(big2Std);           // OOPS:  -1 3 4 5
    print(big2Bel);                    // OK:  9 3 4 5

Note that without the first call of `printUniversal()` the second call would be correct.
Thus, for standard views, even a pure reading iteration might change or even invalidate later behavior.

Caching also has the effect that a filter view has undefined behavior when used in an iteration
that modifies the elements (which is in principle allowed):

    std::vector<int> coll{1, 4, 7, 10};
    auto isEven = [] (auto&& i) { return i % 2 == 0; };

    // standard behavior when incrementing even elements twice:
    auto collEvenStd = coll | std::views::filter(isEven);
    print(coll);    // 1 4 7 10
    for (int& i : collEvenStd) {
      i += 1;       // ERROR (undefined behavior)
    }
    print(coll);    // 1 5 7 11
    for (int& i : collEvenStd) {
      i += 1;       // ERROR (undefined behavior)
    }
    print(coll);    // 1 6 7 11   (note the 6!)

The problem is that the modification is required not to break the predicate of the filter.
Otherwise, the standard views have undefined behavior (and bad behavior as you can see).

Again, belleviews do not have this problem:

    // belleviews behavior when incrementing even elements twice:
    auto collEvenBel = coll | bel::views::filter(isEven);
    print(coll);    // 1 4 7 10
    for (int& i : collEvenBel) {
      i += 1;       // OK
    }
    print(coll);    // 1 5 7 11
    for (int& i : collEvenBel) {
      i += 1;       // OK
    }
    print(coll);    // 1 5 7 11

Belle views support concurrent read iterations for all views:

    std::list coll{1, 2, 3, 4, 5, 6, 7, 8};    // no random-access range

    auto vStd = coll | std::views::drop(2);    // standard drop view
    std::jthread t1{[&] {
      for (const auto& elem : vStd) {          // vStd.begin() in separate thread
        std::cout << elem << '\n';
      }}};
    auto cnt1 = std::ranges::count_if(vStd,    // Runtime ERROR (data race)
                                      [](int i) {return i < 0;});  
    
    auto vBel = coll | bel::views::drop(2);    // belle drop view
    std::jthread t2{[&] {
      for (const auto& elem : vBel) {          // vBel.begin() in separate thread
        std::cout << elem << '\n';
      }}};
    auto cnt2 = std::ranges::count_if(vBel,    // OK
                                      [](int i) {return i < 0;});  

Belle views are more consistent and complete.
For example, you have consistent names of all view types
and an adaptor/factory for all of them:

    std::ranges::subrange s1{++coll.begin(), --coll.end()};    // strange name, no factory

    auto s2 = bel::views::sub(++coll.begin(), --coll.end());   // factory with consistent name

For more examples, see in sources all the test code.


## Design decisions

**Goals:**

- Usability (simplicity, consistency, and a lot of use cases that standard views do (surprisingly) not support should just work)
- Safety (a couple of non-intuitive cases for undefined behavior should no longer occur)
- Performance (the library should stll have good performance)
- Predictability (common use cases should work as expected)

**Principles:**

- **Iterating over a view is stateless**
  - You can always iterate when the view is const
  - You can iterate concurrently
  - Iterating does not have any impact on later behavior (except with modifications, of course)

- **Respect and honor constness**
  - Views always propagate constness (is the view const, the elements are const)
  - Avoid using references that break the effect of declaring elements const

- **A copy of a view always behaves the same as its source**

- **All view types have a name ending with view**
  - Use sub_view instead of subrange

- **Consistent naming**

  - For example, we have the view **`sub_view`** instead of `subrange`.

- **For all view types there is an adaptor/factory** so that you never need to use view types directly

  - New factory bel::views::sub(beg,end) 

- **Fix all specific flaws that create inconsistencies or confusion**

  - E.g., allow elements_view to use any type with a tuple-like API, not just std::tuple and std::pair 

**Open:**

- Shall we stil support implicit conversion to a ref_view?

**Still:**

- Views might be borrowed or might be not
  - We could make views borrowed if they don't own a range, but the question is whether it is worth it

  
## Status

Available:
- `ref_view`, `owning_view`, `all()`, and `all_t`
- `drop_view` and `drop()`
- `take_view` and `take()`
- `filter_view` and `filter()`
- `transform_view` and `transform()`
- `drop_while_view` and `drop_while()`
- `sub_view` (a.k.a. `bel::subrange`) with factory `sub(beg,end)`

### ToDo

OPEN ISSUES (probably a lot but let's start with some):
- const sentinel support
  (why does make_const_iterator() not work for sentinels?)

OPEN TOPICS:
1. Support of counted, common, take_while
1. Support of elements, keys, values
   - with fix that elements always works on tuple-like APIs
1. Support of reverse
1. Support of `zip_view` (fix the new C++23 const nightmare there)


## Tests

See the programs `test*.cpp` in directory sources.

## More

For more details on how to deal with C++20 views
see

>  C++20 - The Complete Guide by Nicolai M. Josuttis
>
>  http://cppstd20.com

## Acknowledgements

This library could not have been implemented without the tremendous work of
the designers and the implementors of the ranges library,
which becamne part of the C++ standard in C++20.
This library is built on top of it.
So many many thanks for all who worked on this libary.

The code of this library is partially a modification of code of the GNU ISO C++ Library.
That code was of tremendous help to make this implementation and I want to thank
all the developers of this library.


## Feedback

I am happy about any constructive feedback.
Please use the feedback address of my C++20 book: http://cppstd20.com/feedback

## License

Because this code uses modified versions of
the GNU ISO C++ Library,
the licence is the follow the GNU GPL.
See the COPYRIGHT file.

