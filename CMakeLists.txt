cmake_minimum_required(VERSION 3.27)
project(OrangePi_SPI_LCD)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
        src/ILI9486.cpp
        main.cpp)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        include)

