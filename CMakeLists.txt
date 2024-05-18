cmake_minimum_required(VERSION 3.12)

project(fix-gateway VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/lib")

add_compile_definitions()
add_compile_options(-pthread)
add_link_options(-pthread)

add_executable(${PROJECT_NAME} src/main.cc)
target_include_directories(${PROJECT_NAME} PRIVATE .. PUBLIC include ${HOME}/include)
target_link_directories(${PROJECT_NAME} PUBLIC lib ${HOME}/lib /usr/lib/x86_64-linux-gnu)
target_link_libraries(${PROJECT_NAME} PUBLIC quickfix)

if (BUILD_TESTS)
	enable_testing()
	add_test(utest-${PROJECT_NAME} utest-${PROJECT_NAME})
	add_executable(utest-${PROJECT_NAME} test/test.cc)
	target_include_directories(utest-${PROJECT_NAME} PRIVATE .. PUBLIC include ${HOME}/include /usr/local/include)
	target_link_directories(utest-${PROJECT_NAME} PUBLIC lib ${HOME}/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
	target_link_libraries(utest-${PROJECT_NAME} PUBLIC gtest gtest_main)
endif()
