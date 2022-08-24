
include(CTest)

set(libs
    pthreadVCE3
    pthreadVSE3
    pthreadVC3
)

macro(add_testcases name srcs)
    foreach(lib ${libs})
        set(test_name test-${name}-${lib})
        add_executable(${test_name} ${srcs})
        target_link_libraries(${test_name} PUBLIC ${lib} ws2_32)
        target_compile_definitions(${test_name} PUBLIC _CONSOLE _MBCS __PtW32NoCatchWarn)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endmacro()

file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)
list(REMOVE_ITEM test_srcs
    "benchlib.c"
    "context2.c"
    "tryentercs2.c"
    "openmp1.c"
)

foreach(test_src ${test_srcs})
    get_filename_component(test_name ${test_src} NAME_WE)

    if(${test_name} MATCHES "benchtest")
        list(APPEND test_src "benchlib.c")
    endif()
    
    add_testcases(${test_name} ${test_src})
endforeach()
