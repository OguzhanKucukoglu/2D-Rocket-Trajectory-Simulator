# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\RocketSimulator_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\RocketSimulator_autogen.dir\\ParseCache.txt"
  "RocketSimulator_autogen"
  )
endif()
