cmake_minimum_required(VERSION 3.16)
project(vdb2raw)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

## OpenVDB support ##
# Monitor <PackageName>_ROOT variables, as this mechanism is used a lot in OpenVDB.
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# Try to use the OpenVDB-provided FindOpenVDB.cmake script.
if (${OPENVDB_ROOT})
  list(PREPEND CMAKE_MODULE_PATH ${OPENVDB_ROOT}/lib/cmake/OpenVDB)
endif()

find_package(OpenVDB 9.0.0 REQUIRED)

find_package(TBB REQUIRED)

add_executable(vdb2raw src/vdb2raw.cxx)
target_link_libraries(vdb2raw PUBLIC OpenVDB::openvdb)
target_link_libraries(${PROJECT_NAME} PUBLIC TBB::tbb)