cmake_minimum_required(VERSION 3.2.0)

project(parallel-writer)

set(CMAKE_CXX_FLAGS "-std=c++14 -fopenmp")

find_package(ParaView 5.6)
include_directories("$ENV{ParaView_DIR}/include/paraview-5.6")
link_directories("$ENV{ParaView_DIR}/lib")

add_executable(parallel-writer parallel-vtk-writer-minimal.cc)
target_link_libraries(parallel-writer "vtkCommonCore;vtkIOParallelXML")
