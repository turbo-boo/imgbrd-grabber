project(sites)

# Ensure npm is available for the sites build steps
if (WIN32)
	find_program(NPM_EXECUTABLE NAMES npm.cmd)
	if (NOT NPM_EXECUTABLE)
		find_program(NPM_EXECUTABLE NAMES npm)
	endif()
else()
	find_program(NPM_EXECUTABLE NAMES npm)
endif()
if (NOT NPM_EXECUTABLE)
	message(FATAL_ERROR "npm not found in PATH")
endif()

# Get all TS files to transpile
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/**.ts")

# Exclude type definitions and node_modules from the list
include(ListFilterRegex)
listFilterRegex(SOURCES ".*.d.ts$")
listFilterRegex(SOURCES "node_modules")

# Install NPM dependencies
add_custom_command(
	OUTPUT NPM_modules
	COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_SOURCE_DIR}" "${NPM_EXECUTABLE}" install
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/NPM_modules
	COMMENT "Installing npm packages..."
	DEPENDS package.json
)
add_custom_target(sites_modules DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/NPM_modules SOURCES package.json)

# Transpiling TypeScript files into JavaScript
add_custom_command(
	OUTPUT JavaScript_sites
	COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_SOURCE_DIR}" "${NPM_EXECUTABLE}" run build
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/JavaScript_sites
	COMMENT "Transpiling TypeScript sources into JavaScript..."
	DEPENDS "${SOURCES}"
)

add_custom_target(sites DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/JavaScript_sites SOURCES "${SOURCES}")
add_dependencies(sites sites_modules)

add_test(
	NAME sites
	COMMAND "${NPM_EXECUTABLE}" run test
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
