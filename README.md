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

## For example

TODO

For the moment see sources/testdrop.cpp

## ToDo

DONE:
- drop_view
- take_view

OPEN:
- take with vector that reallocated memory

1. Support of basic views: drop, take, filter, transform (<font color="red">at work</font>)
1. Why does make_const not work for sentinels?
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


