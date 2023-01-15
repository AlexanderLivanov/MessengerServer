cmake_minimum_required(VERSION 3.10)

project(MyOwnMessengerServer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)


include_directories(include)

#set(OPENSSL_USE_STATIC_LIBS TRUE)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
