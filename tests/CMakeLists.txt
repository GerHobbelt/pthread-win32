
include (CTest)

include_directories(..)

set(XXLIBS	ws2_32.lib)
set(TESTPREFIX  "${CMAKE_INSTALL_PREFIX}/${TARGET_ARCH}")

set(VCEFLAGS "${VCEFLAGS} -D__PtW32NoCatchWarn")
# Remove "-DNDEBUG" from the compiler's list of command line defs.
# This affects the tests' use of the assert macro. This comes from
# the GNUmakefile command line for tests.
string(REPLACE "-DNDEBUG" "-UNDEBUG" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

message(STATUS "Test destination prefix: ${TESTPREFIX}")

macro(add_testcase test type def)
    set(lib_test test-${test}${type}${PTW32_VER})
    set(dll_test test-dll-${test}${type}${PTW32_VER})

    set(lib_lib libpthread${type}${PTW32_VER})
    set(dll_lib pthread${type}${PTW32_VER})

    set(c_dep "")
    if(${test} MATCHES "benchtest")
        set(c_dep "benchlib.c")
    endif()

    set(extra "")
    if(${test} MATCHES "openmp1")
        if(MSVC)
            set(extra "/openmp" "-D_OPENMP")
	elseif (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
            set(extra "-fopenmp")
        endif()
    endif()

    add_executable(${lib_test} ${test}.c ${c_dep})
    target_link_libraries(${lib_test} ${lib_lib} ${XXLIBS})
    if(MSVC)
      target_compile_options(${lib_test} PUBLIC "/nologo")
    endif()
    target_compile_definitions(${lib_test} PUBLIC _CONSOLE _MBCS ${def} ${extras})
    target_compile_options(${lib_test} PUBLIC ${extra})
    if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	# GNUmakefile compiles every test as C++:
	target_compile_definitions(${lib_test} PRIVATE __PtW32NoCatchWarn)
	target_compile_options(${lib_test} PRIVATE "-mthreads" "-xc++")
	target_link_libraries(${lib_test} stdc++)
	target_link_options(${lib_test} PRIVATE "-shared-libgcc")

	if (${test} MATCHES "openmp")
	    target_link_options(${lib_test} PRIVATE "-fopenmp")
	endif ()
    endif ()
    add_dependencies(${lib_test} ${lib_lib})

    add_executable(${dll_test} ${test}.c ${c_dep})
    target_link_libraries(${dll_test} ${dll_lib} ${XXLIBS})
    if(MSVC)
      target_compile_options(${dll_test} PUBLIC "/nologo")
    endif()
    target_compile_definitions(${dll_test} PRIVATE _CONSOLE _MBCS ${def})
    target_compile_options(${dll_test} PRIVATE ${extra})
    if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	# GNUmakefile compiles every test as C++:
	target_compile_definitions(${dll_test} PRIVATE __PtW32NoCatchWarn)
	target_compile_options(${dll_test} PRIVATE "-mthreads" "-xc++")
	target_link_libraries(${dll_test} stdc++)
	target_link_options(${dll_test} PRIVATE "-shared-libgcc")
	if (${test} MATCHES "openmp")
	    target_link_options(${dll_test} PRIVATE "-fopenmp")
	endif ()
    endif ()
    add_dependencies(${dll_test} ${dll_lib})

    if (${CMAKE_GENERATOR} MATCHES "Visual Studio")
        install(
	    FILES "${CMAKE_BINARY_DIR}/tests/$<CONFIG>/${lib_test}.exe"
	    DESTINATION "${TESTPREFIX}/$<CONFIG>/tests"
	)
        install(
	    FILES "${CMAKE_BINARY_DIR}/tests/$<CONFIG>/${dll_test}.exe"
	    DESTINATION "${TESTPREFIX}/$<CONFIG>/tests"
	)
    else()
        install(
	    FILES ${CMAKE_BINARY_DIR}/tests/${lib_test}.exe
	    DESTINATION "${TESTPREFIX}/$<CONFIG>/tests"
	)
        install(
	    FILES ${CMAKE_BINARY_DIR}/tests/${dll_test}.exe
	    DESTINATION "${TESTPREFIX}/$<CONFIG>/tests"
	)
    endif()

    if(${type} MATCHES "VCE")
        set_target_properties(${lib_test} PROPERTIES COMPILE_FLAGS ${VCEFLAGS})
        set_target_properties(${dll_test} PROPERTIES COMPILE_FLAGS ${VCEFLAGS})
    endif()

    add_test(NAME ${lib_test} COMMAND ${lib_test})
    add_test(NAME ${dll_test} COMMAND ${dll_test})

endmacro()


file(GLOB TESTS *.c)

foreach(t ${TESTS})

    get_filename_component(test ${t} NAME)
    string(REPLACE ".c" "" test "${test}")

    # exclusions
    if(${test} STREQUAL "benchlib")
        list(REMOVE_ITEM TESTS ${t})
        continue()
    elseif(${test} STREQUAL "context2")    # SEGFAULT
        continue()
    elseif(${test} STREQUAL "tryentercs2") # SEGFAULT
        continue()
    endif()

    if (MSVC)
	add_testcase(${test} VCE __PTW32_CLEANUP_CXX )
	add_testcase(${test} VSE __PTW32_CLEANUP_SEH )
	add_testcase(${test} VC  __PTW32_CLEANUP_C )
    elseif (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	add_testcase (${test} GCE __PTW32_CLEANUP_CXX )
	add_testcase (${test} GC  __PTW32_CLEANUP_C )
    endif (MSVC)

endforeach(t)
