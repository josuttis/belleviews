# Belle Views for C++

Belleviews - A library of C++ views that are  easier to use, more robust,
and cause less unexpected compile-time and runtime errors than the views of the C++ standard library.

## Why?

C++20 introduced the ranges library which contain views.
Views are lightweight ranges that refer to (a subset of) a range
with optional transformation of the values.

Views are simple and great to use.
However, as standardized they break a couple of basic guarantees collections
such as containers usually have:
- You might not be able to iterate over the elements of a standard view when the view is const.
- As a consequence generic code that for all ranges (coontainers and views)
   has to declare the parameters as universal/forwarding references.
- Standard views do not propagate constness.
   This means that declaring the view const does not declare the elements to be const.
- Iterating over a standard view concurrently with two threads might cause a data race
   (a runtime error due to undefined behavior).
- For that reason, using standard views in parallel algorithms is
   also a runtime error (undefined behavior).
- For some standard views declaring the elements as const does have no effect.
   You might be able to modify the members of a const element of a view.
- Iterations that read elements might affect the functional behavior
   of later iterations.
- Copying a view might create a view that has a different state and behavior than the source view.
- cbegin() and cend() might not make elements const.
- Type const_iterator is available.
- A standard view is not always a pure subset restricting or dealing with the elements of a range,
   but not providing opeionts the range would not provide.
   They might remove constraints and provide options, the range itself does not have. 

This make the use of views error-prone, confusing and programmers might easily create fatal runtime errors
wihtout being noticing it.

There are multiple reasons for this behavior:
- Views might cache begin to oiptimize for multiple iterations with the same view
   (the funny part is that due to its restrictions, it is recommended to use them ad-hoc
    and iterate only once).
- The designers of the views considered views as pointers instead of
   references to collections. That especially causes the confusing handling of const.

We can do better.
This library provides views that are simple to use and reducse the number of suprises causing compiletime errpors and runtime errorsy.
In some cases the price may be a worse performance.
However, programs are far mor predictable in behavior, do what programmers expect, generic code
using them becomes way more cimpler and you can still get the well performance with some additional
tricks.

## How to use Belle Views

All you hav to do is to use the namespace bel::views instead of std::views.

## Examples

Belle views can always iterate over elements even if the views are const:

    template<typename T>
    void print(const T& coll);           // print elements of a collection
    
    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8};
    std::list lst{1, 2, 3, 4, 5, 6, 7, 8};

    print(vec | std::views::drop(2));    // OK
    print(lst | std::views::drop(2));    // ERROR with standard views
    print(vec | bel::views::drop(2));    // OK
    print(lst | bel::views::drop(2));    // OK with belle views

Belle views propagate const:

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

Note that without the first call of printUniversal() the second call would be correct.
Thus, for standard views, a read iteration might change or veen invalidate later behavior.

Belle views support concurrent read iterations for all views:

    std::vector vec{1, 2, 3, 4, 5, 6, 7, 8};

    auto vStd = vec | std::views::drop(2);
    auto sum1 = std::reduce(std::execution::par,      // RUNTIME ERROR (possible data race)
                            vStd.begin(), vStd.end(),
                            0L);
    auto vBel = vec | bel::views::drop(2);
    auto sum2 = std::reduce(std::execution::par,      // OK
                            vBel.begin(), vBel.end(),
                            0L);

For more examples, see in sources: testdrop.cpp, testtake.cpp, testfilter.cpp

## ToDo

AVAILABLE:
- drop_view
- take_view
- filter_view

OPEN ISSUES:
- const sentinel support
  (why does make_const_iterator() not work for sentinels?)
- make using std views with bel::filter compile

OPEN TOPICS:
1. Support of remaining basic views: transform (<font color="red">at work</font>)
1. Support of all, counted, common
1. Support of elements, keys,values
   - with fix that elements always works on tuple-like APIs
1. Support of reverse
1. Additional pipe begin ?


## Tests

TODO

## More

For more details on how to deal with C++20 views
see

>  C++20 - The Complete Guide by Nicolai M. Josuttis
>
>  http://cppstd20.com

## Feedback

I am happy about any constructive feedback.
Please use the feedback address of my C++20 book: http://cppstd20.com/feedback

## License

The code is licensed under a Creative Commons Attribution 4.0 International License.

http://creativecommons.org/licenses/by/4.0/


