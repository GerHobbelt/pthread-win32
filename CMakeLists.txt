cmake_minimum_required(VERSION 3.8)
project(pthread)

# STATIC library pthread
set(PUBLIC_INTERFACE pthread.h sched.h semaphore.h)
file(GLOB PRIVATE_HEADERS *.h)
file(GLOB PRIVATE_SOURCES  *.c)
source_group(include FILES ${PRIVATE_HEADERS})
source_group(src     FILES ${PRIVATE_SOURCES})

include_directories(".")
add_library(pthread STATIC ${PRIVATE_HEADERS} ${PRIVATE_SOURCES})
target_compile_definitions(pthread PRIVATE
    -DPTW32_STATIC_LIB=1
    -DHAVE_CONFIG_H=1
)

if(MSVC)
  add_compile_options(/Gm-)
  add_link_options(/ignore:4006)
endif()

install(FILES ${PUBLIC_INTERFACE} DESTINATION include)
install(TARGETS pthread
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
