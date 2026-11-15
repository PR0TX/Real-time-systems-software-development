cmake_minimum_required(VERSION 3.16)
project(lab3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)
find_package(controller_interface REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(smacc2 REQUIRED)
find_package(std_srvs REQUIRED)

foreach(node bed_service bed_fsm bed_bt simulate_joint)
  add_executable(${node} src/${node}.cpp)
  target_link_libraries(${node} PRIVATE rclcpp::rclcpp)
  install(TARGETS ${node} DESTINATION lib/${PROJECT_NAME})
endforeach()

foreach(node bed_service bed_fsm bed_bt)
  target_link_libraries(${node} PRIVATE std_srvs::std_srvs__rosidl_typesupport_cpp)
endforeach()

target_include_directories(bed_fsm SYSTEM PRIVATE ${smacc2_INCLUDE_DIRS})
target_link_libraries(bed_fsm PRIVATE ${smacc2_LIBRARIES} Boost::thread)
target_link_libraries(bed_bt PRIVATE behaviortree_cpp::behaviortree_cpp ament_index_cpp::ament_index_cpp)

add_library(lab3_control SHARED src/joint_control.cpp)
target_link_libraries(lab3_control PRIVATE controller_interface::controller_interface
  hardware_interface::hardware_interface pluginlib::pluginlib)
pluginlib_export_plugin_description_file(hardware_interface config/hardware_plugin.xml)
pluginlib_export_plugin_description_file(controller_interface config/controller_plugin.xml)
install(TARGETS lab3_control LIBRARY DESTINATION lib)
install(DIRECTORY config launch DESTINATION share/${PROJECT_NAME})
install(PROGRAMS scripts/joint_experiments.py DESTINATION lib/${PROJECT_NAME} RENAME joint_experiments)

ament_package()
