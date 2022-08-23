
include (CTest)

macro(add_testcase t lib)
    set(test_name test-${t}-${lib})
    add_executable(${test_name} ${t}.c)
    target_link_libraries(${test_name} PUBLIC ${lib})
    target_compile_definitions(${test_name} PUBLIC _CONSOLE _MBCS __PtW32NoCatchWarn)
    target_compile_options(${test_name} PUBLIC "/nologo")
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

file(GLOB test_srcs *.c)
list(REMOVE_ITEM test_srcs "benchlib.c" "context2.c" "tryentercs2.c" "openmp1.c")

foreach(test_src ${test_srcs})
    get_filename_component(t ${test_src} NAME_WE)
    add_testcase(${t} pthreadVCE3)
    add_testcase(${t} pthreadVSE3)
    add_testcase(${t} pthreadVC3)
endforeach()
