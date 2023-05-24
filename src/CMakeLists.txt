add_definitions(-DSYSTEMTYPE="Linux")

file(STRINGS extrasources.make project_FILES)   # AK: WHY this way?
foreach(fil ${project_FILES})
  string(REPLACE " " "" fil ${fil})
  if(NOT ${fil} MATCHES main.cpp)
   list(APPEND project_SRC ${fil})
  endif(NOT ${fil} MATCHES main.cpp)
endforeach(fil)


# set the default build type to 'debug'.
SET(CMAKE_BUILD_TYPE Debug CACHE STRING "default to debug" FORCE)

add_executable(${PROJECT}-opt ${project_SRC} main)
target_link_libraries(${PROJECT}-opt "${LIBS}")
set_target_properties(${PROJECT}-opt PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC -DNDEBUG -O3")

add_executable(${PROJECT}-debug ${project_SRC} main)
target_link_libraries(${PROJECT}-debug "${LIBS}")
set_target_properties(${PROJECT}-debug PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")

