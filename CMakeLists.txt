cmake_minimum_required(VERSION 3.28)

#project(ParProc CXX)
#
#set(CMAKE_CXX_STANDARD 20)
#
#find_package(MPI REQUIRED)
##include_directories(SYSTEM ${MPI_INCLUDE_PATH})
#
#find_package(OpenMP REQUIRED)
#add_executable(Lab7 l/lab7.cpp)
#
#if (OPENMP_FOUND)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#    target_link_libraries(Lab7 PRIVATE ${OpenMP_CXX_LIBRARIES})
#endif()
#
#
#target_link_libraries(Lab7 PRIVATE MPI::MPI_CXX)
project(ParProc C)
#project(ParProc CXX)

set(CMAKE_C_STANDARD 17)
#set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)
#include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(ParProc main.c)
#add_executable(ParProc main.cpp)
#add_executable(Lab1 Lab1/lab1.c)
add_executable(Lab2 Lab2/lab2_add.c)
#add_executable(Lab3 Lab3/lab3_forshow2.c)
add_executable(Lab4 Lab4/lab4_forshow.c)
add_executable(Lab5 Lab5/lab5.c)
#add_executable(Lab23 Lab3/test2.c)
#add_executable(Lab33 Lab3/lab3_2it.c)
#add_executable(Lab2notmine "Lab2notmine/lab2(1).c")
#target_link_libraries(Lab3 PRIVATE MPI::MPI_C)
#add_executable(Lab1notmine "Lab1/lab1_size_dependency.c")
#add_compile_options("-Ofast")
if (OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    #message(WARNING ${CMAKE_C_FLAGS})
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    target_link_libraries(Lab5 PRIVATE ${OpenMP_C_LIBRARIES})
    target_link_libraries(Lab2 PRIVATE ${OpenMP_C_LIBRARIES})
    #    target_link_libraries(Lab5 PRIVATE ${OpenMP_CXX_LIBRARIES})
endif ()

target_link_libraries(Lab5 PRIVATE MPI::MPI_C)
#target_link_libraries(Lab5 PRIVATE MPI::MPI_CXX)
