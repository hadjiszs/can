cmake_minimum_required(VERSION 2.8.6)

set(CMAKE_CXX_COMPILER mpic++)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/test)

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

file(GLOB list_unit_test
  "*.cpp"
  )

foreach(FILE ${list_unit_test})
  # recupere le chemin de fichier sans l'extension
  get_filename_component(FILE_WE ${FILE} NAME_WE)

  message(STATUS "\t${FILE_WE}")
  
  add_executable(${FILE_WE} ${FILE})
  message(STATUS "\t\t${MPI_LIBRARIES}")
  target_link_libraries (${FILE_WE} can ${MPI_LIBRARIES})
  add_test(${FILE_WE} ${FILE_WE})
endforeach(FILE)

