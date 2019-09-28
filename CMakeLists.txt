cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Release)
project(SCUT_RoboLab_cv_armor)

find_package(OpenCV REQUIRED)

link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})

add_executable(SCUT_RoboLab_cv_armor SCUT_RoboLab_cv_armor.cpp)
target_link_libraries(SCUT_RoboLab_cv_armor ${OpenCV_LIBRARIES})