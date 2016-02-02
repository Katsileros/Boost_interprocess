#MESSAGE( STATUS )
#MESSAGE( STATUS "==============BOOST Interprocess CMAKELISTS.TXT ==============")
#MESSAGE( STATUS )

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Project name
SET( PROJECT_NAME "boost_interprocess" )
PROJECT( ${PROJECT_NAME} )
SET( PROJECT_DIR ${PROJECT_SOURCE_DIR} )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(include)

find_package(Boost 1.54.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIR})
endif()

add_executable(ipc_0 src/ipc_0.cpp)
add_executable(ipc_1 src/ipc_1.cpp)
target_link_libraries(ipc_0 ${Boost_LIBARIES} boost_system rt pthread boost_thread)
target_link_libraries(ipc_1 ${Boost_LIBARIES} boost_system rt pthread boost_thread)

#MESSAGE( STATUS )
#MESSAGE( STATUS "==============BOOST Interprocess CMAKELISTS.TXT EXITING==============")
#MESSAGE( STATUS )
