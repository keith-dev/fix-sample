cmake_minimum_required(VERSION 3.12)

project(fix-apps VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "$<0:>${CMAKE_BINARY_DIR}/lib")

add_compile_definitions()
add_compile_options(
	-pthread
	-Wno-ignored-qualifiers
	-Wno-unused-private-field
	-Wno-unused-parameter
	-Wno-missing-field-initializers
	-Wno-overloaded-virtual
	-Wno-unused-variable
	-Wno-deprecated-copy-with-user-provided-copy)
add_link_options(-pthread)

add_executable(fix-gateway
	fix-gateway/src/main.cc
	fix-gateway/src/Application.cc
	fix-gateway/src/Model.cc
	fix-gateway/src/Router.cc)
target_include_directories(fix-gateway PRIVATE fix-gateway/include PUBLIC fix-gateway/include ${HOME}/include)
target_link_directories(fix-gateway PUBLIC lib ${HOME}/lib /usr/lib/x86_64-linux-gnu)
target_link_libraries(fix-gateway PUBLIC quickfix)

add_executable(fix-client
	fix-client/src/main.cc
	fix-client/src/Application.cc
	fix-client/src/Router.cc)
target_include_directories(fix-client PRIVATE fix-client/include PUBLIC fix-client/include ${HOME}/include)
target_link_directories(fix-client PUBLIC lib ${HOME}/lib /usr/lib/x86_64-linux-gnu)
target_link_libraries(fix-client PUBLIC quickfix)

if (BUILD_TESTS)
	enable_testing()
	add_test(utest-fix-gateway utest-fix-gateway)
	add_executable(utest-fix-gateway test/test.cc)
	target_include_directories(utest-fix-gateway PRIVATE .. PUBLIC include ${HOME}/include /usr/local/include)
	target_link_directories(utest-fix-gateway PUBLIC lib ${HOME}/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
	target_link_libraries(utest-fix-gateway PUBLIC gtest gtest_main)
endif()
