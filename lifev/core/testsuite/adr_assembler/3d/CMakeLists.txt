INCLUDE(TribitsAddExecutableAndTest)
INCLUDE(TribitsCopyFilesToBinaryDir)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  ADRAssembler3D
  SOURCES main.cpp
  ARGS -c
  NUM_MPI_PROCS 3
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

TRIBITS_COPY_FILES_TO_BINARY_DIR(data_ADRAssembler3D
  SOURCE_FILES data
  CREATE_SYMLINK
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
)
