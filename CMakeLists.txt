cmake_minimum_required(VERSION 3.8)
project(hobot_bev)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV 4 REQUIRED)

set(DEPS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../bev_core/deps_aarch64)
set (SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../bev_core/code/)

message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("DEPS_ROOT: ${DEPS_ROOT}")

# libdnn.so depends on system software dynamic link library, use -Wl,-unresolved-symbols=ignore-in-shared-libs to shield during compilation
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-unresolved-symbols=ignore-in-shared-libs")

if(DEFINED ENV{ROS_DISTRO})
    set(PREFERRED_ROS_DISTRO $ENV{ROS_DISTRO})
else()
    if(EXISTS "/opt/ros/jazzy")
        set(PREFERRED_ROS_DISTRO jazzy)
    elseif(EXISTS "/opt/ros/humble")
        set(PREFERRED_ROS_DISTRO humble)
    else()
        set(PREFERRED_ROS_DISTRO humble)
    endif()
endif()
if(DEFINED PREFERRED_ROS_DISTRO)
    message(STATUS "Preferred ROS distribution: ${PREFERRED_ROS_DISTRO}")
endif()
# if("${PREFERRED_ROS_DISTRO}" STREQUAL "jazzy")
#     add_definitions(-DCV_BRIDGE_CPP)
# endif()



if(PLATFORM_S100)
  message("build ${PROJECT_NAME} for platform S100")
  # set(SYS_ROOT ${CMAKE_SYSROOT})
  set(PREFIX_PATH s100)  
elseif(PLATFORM_S600)
  message("build ${PROJECT_NAME} for platform S600")
  # set(SYS_ROOT ${CMAKE_SYSROOT})
  set(PREFIX_PATH s600)
else()
  message("invalid platform, build platform S100 default")
  # set(SYS_ROOT ${CMAKE_SYSROOT})
  set(PREFIX_PATH s100)
endif()

message("CMAKE_SYSROOT is " ${CMAKE_SYSROOT})

# 头文件包含目录
include_directories(
  ${SRC_PATH}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/deps_aarch64/include/
)

# 链接库目录（动态库.so/静态库.a，指定搜索路径）============================================修改
link_directories(
  ${CMAKE_SYSROOT}/usr/hobot/lib/
  ${CMAKE_CURRENT_SOURCE_DIR}/deps_aarch64/lib/${PREFIX_PATH}/
)

if (DSP_ON)
  add_definitions(-DHAVE_DSP)
endif()

set(os_base_lib pthread rt dl)

SET(Link_libs
        dnn
        gflags
        glog
        hbucp
        tros_ai_wrapper
        ${os_base_lib})

add_executable(${PROJECT_NAME}
        # ${SRC_PATH}/src/simple_example.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bev_node.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
        )

add_executable(gen_scenes
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gen_scenes.cc
)

add_executable(layout_imgs
  ${CMAKE_CURRENT_SOURCE_DIR}/src/layout_imgs.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
)

ament_target_dependencies(layout_imgs
  rclcpp 
  OpenCV
)

if(DSP_ON)
  target_link_libraries(${PROJECT_NAME} ${Link_libs} hbdsp_plugin)
else()
  target_link_libraries(${PROJECT_NAME} ${Link_libs})
endif()

ament_target_dependencies(${PROJECT_NAME} 
  rclcpp 
  sensor_msgs 
  OpenCV
)

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  ${PROJECT_SOURCE_DIR}/config/
  DESTINATION lib/${PROJECT_NAME}/config/
)

install(DIRECTORY
  ${PROJECT_SOURCE_DIR}/deps_aarch64/lib/${PREFIX_PATH}/
  DESTINATION lib/${PROJECT_NAME}/
)

install(DIRECTORY
  ${PROJECT_SOURCE_DIR}/launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
