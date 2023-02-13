if (BELLEVIEW_WITH_TESTS)
    add_executable(testall testall.cpp)
    add_test(NAME testall COMMAND testall)

    add_executable(testdrop testdrop.cpp)
    add_test(NAME testdrop COMMAND testdrop)

    add_executable(testdropwhile testdropwhile.cpp)
    add_test(NAME testdropwhile COMMAND testdropwhile)

    add_executable(testfilter testfilter.cpp)
    add_test(NAME testfilter COMMAND testfilter)

    add_executable(testsub testsub.cpp)
    add_test(NAME testsub COMMAND testsub)

    add_executable(testtake testtake.cpp)
    add_test(NAME testtake COMMAND testtake)

    add_executable(testtransform testtransform.cpp)
    add_test(NAME testtransform COMMAND testtransform)
endif(BELLEVIEW_WITH_TESTS)
