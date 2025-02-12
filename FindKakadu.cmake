set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)

set(KAKADU_VERSION 84)

set(kdu_lib_path "${KAKADU_DIR}/lib/Linux-x86-64-gcc/" "${KAKADU_DIR}/lib/Mac-arm-64-gcc/" "${KAKADU_DIR}/lib/Mac-x86-64-gcc/" "${KAKADU_DIR}/../bin_x64/" "${KAKADU_DIR}/../bin_x86/")

set(kdu_inc_path "${KAKADU_DIR}/managed/all_includes/")

find_path(kdu_inc jp2.h PATHS ${kdu_inc_path})

if(WIN32)
	set(kdu_lib_name "kdu_v${KAKADU_VERSION}R")
	set(kdu_aux_name "kdu_a${KAKADU_VERSION}R")
else()
	set(kdu_lib_name "kdu")
	set(kdu_aux_name "kdu_aux")
endif()

find_library(kdu_lib NAMES ${kdu_lib_name} PATHS ${kdu_lib_path} NO_DEFAULT_PATH)
find_library(kdu_aux NAMES ${kdu_aux_name} PATHS ${kdu_lib_path} NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Kakadu DEFAULT_MSG kdu_lib kdu_aux kdu_inc)

if(kdu_inc AND kdu_lib AND kdu_aux)
  set(KAKADU_FOUND TRUE)

  add_library(KDU STATIC IMPORTED)
  add_library(KDU_AUX STATIC IMPORTED)

  #set(KAKADU_LIBRARIES ${kdu_lib_path} ${kdu_aux_path})
  #set(KAKADU_INCLUDE_DIRS ${kdu_inc})
  	
  set_target_properties(KDU PROPERTIES IMPORTED_LOCATION ${kdu_lib} INTERFACE_INCLUDE_DIRECTORIES ${kdu_inc})
  set_target_properties(KDU_AUX PROPERTIES IMPORTED_LOCATION ${kdu_aux} INTERFACE_INCLUDE_DIRECTORIES ${kdu_inc})

  message(STATUS "Kakadu libraries = ${kdu_lib}, ${kdu_aux}")
  message(STATUS "Kakadu include = ${kdu_inc}") 

endif()