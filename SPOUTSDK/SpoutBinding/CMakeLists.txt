#\-------------------------------------- . -----------------------------------/#
# Filename : CMakeLists.txt              | SpoutBinding CMakeList              #
#/-------------------------------------- . -----------------------------------\#
#                                                                              #
#  C ABI wrapper library exposing minimal Spout sender functions via           #
#  extern "C" for Java FFM (jextract) bindings.                               #
#                                                                              #
#/-------------------------------------- . -----------------------------------\#

cmake_minimum_required(VERSION 3.15)
project(SpoutBinding)

set(SpoutBinding_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutBinding.cpp
)
set(SpoutBinding_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/SpoutBinding.h
)

#-----------------------------
# SpoutBinding shared DLL
#
add_library(SpoutBinding SHARED ${SpoutBinding_SOURCES} ${SpoutBinding_HEADERS})

# Define SPOUTBINDING_EXPORTS when building the DLL so that
# SPOUTBINDING_API expands to __declspec(dllexport).
target_compile_definitions(SpoutBinding PRIVATE SPOUTBINDING_EXPORTS)

# Link against the static Spout core library.
# Spout_static provides the Spout C++ class and all its dependencies
# (OpenGL, DirectX, shared memory, etc.).
target_link_libraries(SpoutBinding PUBLIC Spout_static)

# Include directories for consuming code:
#   BUILD_INTERFACE  - consumers add SpoutBinding/ to their include path
#   INSTALL_INTERFACE - installed location under CMAKE_INSTALL_INCLUDEDIR
target_include_directories(SpoutBinding
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(SpoutBinding PROPERTIES
	VS_FOLDER "SpoutBinding"
	SPOUT_PUBLIC_HEADER "${SpoutBinding_HEADERS}"
	RUNTIME_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/bin
	LIBRARY_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
	ARCHIVE_OUTPUT_DIRECTORY ${SPOUT_BINARY_DIR}/lib
)

#/--------------------------------------
# SpoutBinding install
#
if (NOT SKIP_INSTALL_ALL)
	if(NOT SKIP_INSTALL_LIBRARIES)
		install(
			TARGETS SpoutBinding
			EXPORT SpoutTargets
			ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
			LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
			RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		)
	endif(NOT SKIP_INSTALL_LIBRARIES)
	if(NOT SKIP_INSTALL_HEADERS)
		install(
			FILES ${SpoutBinding_HEADERS}
			DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SpoutBinding
		)
	endif(NOT SKIP_INSTALL_HEADERS)
endif (NOT SKIP_INSTALL_ALL)
