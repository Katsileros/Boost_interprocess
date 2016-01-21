#MESSAGE( STATUS )
#MESSAGE( STATUS "==============BOOST Interprocess CMAKELISTS.TXT ==============")
#MESSAGE( STATUS )

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Project name
SET( PROJECT_NAME "boost_interprocess" )
PROJECT( ${PROJECT_NAME} )
SET( PROJECT_DIR ${PROJECT_SOURCE_DIR} )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Boost 1.54.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set( SRC src/boost_interprocess.cpp)
add_executable(inproc ${SRC})
target_link_libraries(inproc ${Boost_LIBARIES} rt pthread)

#MESSAGE( STATUS )
#MESSAGE( STATUS "==============BOOST Interprocess CMAKELISTS.TXT EXITING==============")
#MESSAGE( STATUS )
