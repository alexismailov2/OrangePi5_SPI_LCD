cmake_minimum_required(VERSION 3.27)
project(OrangePi_SPI_LCD)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
        src/ILI9486.cpp
        main.cpp)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        include
        wiringOP/wiringPi)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/wiringOP/wiringPi/libwiringPi.so.2.52)

