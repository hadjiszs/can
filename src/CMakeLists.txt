cmake_minimum_required(VERSION 2.8)

# ${list_src} contiendra toutes les .cpp du repertoire courant ( src/ )
file(GLOB_RECURSE list_src
  "*.cpp"
  )

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

# on enlève main.cpp contenu dans ${list_src}
file(GLOB to_remove main.cpp)
list(REMOVE_ITEM list_src ${to_remove})

# librairie
add_library(can ${list_src})

# executable
add_executable(exec main)

set_target_properties(can PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(exec can ${MPI_LIBRARIES})
