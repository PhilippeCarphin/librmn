# The minimum version can not be 3.16 because of the LINK_LANGUAGE generator expression in Config.cmake.in
cmake_minimum_required(VERSION 3.20)

message(STATUS "(EC) Generating librmn Makefile")

# Append EC specific module path
list(APPEND CMAKE_MODULE_PATH $ENV{EC_CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_rpn/modules)

include(ec_init)           # Initialise compilers and ec specific functions
ec_git_version()           # Get version from git state
ec_parse_manifest()        # Parse MANIFEST file

project(${NAME} DESCRIPTION "${DESCRIPTION}" LANGUAGES C Fortran)
set(PROJECT_VERSION ${VERSION}${STATE})
set(rmn_VERSION ${PROJECT_VERSION} CACHE INTERNAL "rmn version" FORCE) # Needed for cascaded version identification
message(STATUS "(EC) ${PROJECT_NAME} version = ${PROJECT_VERSION}")

ec_build_info()            # Generate build information

include(ec_compiler_presets)

# Need to add HAVE_RMN definition for auto-registering librmn within App
add_compile_definitions(HAVE_RMN=\"${PROJECT_VERSION}\")
add_subdirectory(App App)

set(WITH_OPENMP TRUE CACHE BOOL "Control whether to use OpenMP")
include(ec_openmp)          # Enable OpenMP (-DWITH_OPENMP=TRUE)
include(ec_doxygen)         # Doxygen target doc

# Fortran compiler options required to build this project
set(Intel_FFLAGS -convert big_endian -align array32byte -assume byterecl)
set(GNU_FFLAGS -fconvert=big-endian -fcray-pointer -frecord-marker=4 -fno-second-underscore)
set(Flang_FFLAGS -Mbyteswapio)
set(PGI_FFLAGS -byteswapio)

# This is required when using GNU compilers without OpenMP.
# It has no adverse effect with other compilers and/or when OpenMP is enabled.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ldl -pthread")
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -ldl -pthread")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -ldl -pthread")

find_package(MPI)
#set(CMAKE_Fortran_COMPILER ${MPI_Fortran_COMPILER})
#set(CMAKE_C_COMPILER ${MPI_C_COMPILER})

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_definitions(_${CMAKE_SYSTEM_NAME}_ _GNU_SOURCE)

option(EXTRA_DEBUG_OUTPUT "Enable extra debug output" OFF)
if(EXTRA_DEBUG_OUTPUT)
    add_compile_definitions(DEBUG)
endif()

# Figure out the correct arch_dependent include path
set(BUILD_ARCH "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Build architecture: ${BUILD_ARCH}")

# TODO Adapt Config.cmake.in to be ready for this
#set(CMAKE_DEBUG_POSTFIX "d")

include_directories(
   include
   src/PUBLIC_INCLUDES
   src/PUBLIC_INCLUDES/rmn
)

file(GLOB PROJECT_C_FILES
   src/*/*.c
)
file(GLOB PROJECT_F_FILES
   src/*/*.F90
   src/*/*.F
   src/*/*.f
   src/*/*.f90
)
file(GLOB PROJECT_H_FILES
   src/*/*.h
   src/*/*.inc
   src/*/*.hf
   src/*/*.cdk
   src/*/*.tpl*
)

# This really isn't the ideal solution; the compiler should be called with all
# the definitions and options as if it was processing a source file

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/rmn)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# App package module directory
get_property(moduleDir TARGET App PROPERTY Fortran_MODULE_DIRECTORY)
include_directories(${moduleDir})

# We need to create a file here because gfortran won't process a file nammmed *.hf
file(GLOB WHITEBOARD_HEADER_DEPS ${CMAKE_CURRENT_SOURCE_DIR}/src/whiteboard/*.tmpl90 ${CMAKE_CURRENT_SOURCE_DIR}/src/whiteboard/*.hf)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/rmn/WhiteBoard.hf
    PRE_BUILD
    COMMAND sed -e "'s/ \"/ </g'" -e "'s/\"/>/g'" ${CMAKE_CURRENT_SOURCE_DIR}/src/whiteboard/WhiteBoard.hf > ${CMAKE_CURRENT_BINARY_DIR}/whiteboard_header.F90
    COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/WhiteBoard.hf
    COMMAND cp -av ${CMAKE_CURRENT_SOURCE_DIR}/src/whiteboard/WhiteBoard_constants.h ${CMAKE_CURRENT_BINARY_DIR}/include/rmn/WhiteBoard.hf
    COMMAND ${CMAKE_Fortran_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}/include -I${CMAKE_CURRENT_SOURCE_DIR}/src/PUBLIC_INCLUDES -I${CMAKE_CURRENT_SOURCE_DIR}/src/whiteboard -P -E ${CMAKE_CURRENT_BINARY_DIR}/whiteboard_header.F90 >> ${CMAKE_CURRENT_BINARY_DIR}/include/rmn/WhiteBoard.hf
    COMMENT "Generating WhiteBoard.hf"
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/src/whiteboard/WhiteBoard.hf
    DEPENDS ${WHITEBOARD_HEADER_DEPS}

    USES_TERMINAL
)
# Add a custom target to regenerate WhiteBoard.hf
add_custom_target(WhiteBoardFortranHeader DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/rmn/WhiteBoard.hf)

string(REGEX MATCH "([0-9]+)" SOVERSION ${PROJECT_VERSION})

add_library(rmn STATIC ${PROJECT_C_FILES} ${PROJECT_F_FILES})
set_target_properties(rmn PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/PUBLIC_INCLUDES/rmn.h
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME rmn)
target_link_libraries(rmn PUBLIC App)

add_library(rmnshared SHARED ${PROJECT_C_FILES} ${PROJECT_F_FILES})
set_target_properties(rmnshared PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/PUBLIC_INCLUDES/rmn.h
    POSITION_INDEPENDENT_CODE ON
    SOVERSION ${SOVERSION}
    OUTPUT_NAME rmn)
target_link_libraries(rmnshared PUBLIC App)

# Required to prevent missing Fortran modules when building in parallel
# (make -j) with GNU compilers
add_dependencies(rmn App)
add_dependencies(rmnshared rmn)

set(TARGETS rmn rmnshared)

if(OpenMP_FOUND)
   set(TARGETS rmn rmnshared rmn-ompi rmnshared-ompi)
   add_library(rmn-ompi STATIC ${PROJECT_C_FILES} ${PROJECT_F_FILES})
   add_library(rmnshared-ompi SHARED ${PROJECT_C_FILES} ${PROJECT_F_FILES})
   add_dependencies(rmn-ompi App-ompi)
   add_dependencies(rmnshared-ompi rmn-ompi)

   set_target_properties(rmn-ompi PROPERTIES
      VERSION ${PROJECT_VERSION}
      PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/PUBLIC_INCLUDES/rmn.h
      POSITION_INDEPENDENT_CODE ON
      OUTPUT_NAME rmn-ompi)
   target_link_libraries(rmn-ompi PUBLIC App-ompi OpenMP::OpenMP_C OpenMP::OpenMP_Fortran MPI::MPI_C MPI::MPI_Fortran)
   target_compile_definitions(rmn-ompi PUBLIC HAVE_MPI HAVE_OPENMP)

   set_target_properties(rmnshared-ompi PROPERTIES
      VERSION ${PROJECT_VERSION}
      PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/PUBLIC_INCLUDES/rmn.h
      POSITION_INDEPENDENT_CODE ON
      SOVERSION ${SOVERSION}
      OUTPUT_NAME rmn-ompi)
   target_link_libraries(rmnshared-ompi PUBLIC App-ompi OpenMP::OpenMP_C OpenMP::OpenMP_Fortran MPI::MPI_C MPI::MPI_Fortran)
   target_compile_definitions(rmnshared-ompi PUBLIC HAVE_MPI HAVE_OPENMP)

   # Symbolic link for archive numbered version
   add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
       COMMAND ln -sfv lib${PROJECT_NAME}-ompi.a lib${PROJECT_NAME}-ompi_${PROJECT_VERSION}.a)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}-ompi_${PROJECT_VERSION}.a DESTINATION lib)
endif()

foreach(TARGET IN ITEMS ${TARGETS})
    target_compile_options(${TARGET}
        PUBLIC $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:${GNU_FFLAGS}>
               $<$<COMPILE_LANG_AND_ID:Fortran,Intel>:${Intel_FFLAGS}>
               $<$<COMPILE_LANG_AND_ID:Fortran,Flang>:${Flang_FFLAGS}>
               $<$<COMPILE_LANG_AND_ID:Fortran,PGI>:${PGI_FFLAGS}>
               $<$<COMPILE_LANG_AND_ID:Fortran,NVHPC>:${PGI_FFLAGS}>)
    add_dependencies(${TARGET}
        ${PROJECT_NAME}_build_info
        WhiteBoardFortranHeader)
endforeach()

install(TARGETS ${TARGETS})

# Symbolic link for archive numbered version
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ln -sfv lib${PROJECT_NAME}.a lib${PROJECT_NAME}_${PROJECT_VERSION}.a)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_${PROJECT_VERSION}.a DESTINATION lib)

# Fortran modules are installed in the include directory for now
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/rmn_jar.mod
    ${CMAKE_CURRENT_BINARY_DIR}/rmn_gmm.mod
    DESTINATION include)

file(GLOB PROJECT_INCLUDE_FILES src/PUBLIC_INCLUDES/rmn/*)
# WhiteBoard.hf is generated by make and doesn't exist when cmake is executed
# It must therefore be added explicitly
list(APPEND PROJECT_INCLUDE_FILES ${CMAKE_CURRENT_BINARY_DIR}/include/rmn/WhiteBoard.hf)
install(FILES ${PROJECT_INCLUDE_FILES} DESTINATION include/rmn)
add_custom_target(
    cprotos
    ${CMAKE_CURRENT_SOURCE_DIR}/extractProtos.bash ${CMAKE_CURRENT_SOURCE_DIR}/src ${PROJECT_C_FILES}
    #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Extract function prototypes from C source code"
)

# Generate the config file for the project to be usable via cmake's find_package command
set(INCLUDE_INSTALL_DIR include)
set(LIB_INSTALL_DIR lib)
set(CONFIG_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}")

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
    PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CONFIG_INSTALL_DIR}"
)

# Packaging
ec_package_name()                 # Define package prefix  
ec_build_config()                 # Create build configuration script
ec_prepare_ssm()                  # Prepare ssm packaging files

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_VENDOR "ECCC")
set(CPACK_PACKAGE_CONTACT "${MAINTAINER}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/package")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${NAME}_${PROJECT_VERSION}")
include(CPack)

add_subdirectory(tests)

