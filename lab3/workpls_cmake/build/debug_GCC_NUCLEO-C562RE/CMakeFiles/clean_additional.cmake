# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "debug_GCC_NUCLEO-C562RE")
  file(REMOVE_RECURSE
  "workpls.elf"
  "workpls.map"
  )
endif()
