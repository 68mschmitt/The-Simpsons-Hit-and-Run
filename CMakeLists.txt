# cmake --install is part of the supported Linux quickstart.
cmake_minimum_required(VERSION 3.15)

project(SRR2)

# The native Linux install is intentionally small: the retail game data stays
# outside the package and is selected by the installed launcher.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	include(GNUInstallDirs)
endif()

set(SRR2_TITLE "Simpsons Hit & Run")
set(SRR2_VERSION "00.61")
set(SRR2_FILENAME "Simpsons")
set(SRR2_REGION "NTSC" CACHE STRING "Select PAL for the EFIGS multi-language version")
set_property(CACHE SRR2_REGION PROPERTY STRINGS "NTSC" "PAL")

option(SRR2_USE_DLMALLOC "Use Doug Lea's memory allocator for fixed-size heaps" OFF)
option(SRR2_REROUTE_ALL_HEAP_ALLOCATIONS "Reroute all memory allocations by new operator to internal heaps" OFF)
option(SRR2_USE_BINK "Use RAD Game Tools Bink SDK instead of FFmpeg" OFF)
option(SRR2_BUILD_TESTS "Build sample programs for internal libraries" ON)
option(SRR2_MICROPROFILE "Use microprofile instead of the internal profiler" OFF)
option(SRR2_P3D_CG "Use CG shaders rather than GLSL for OpenGL ES" OFF)
option(SRR2_BUILD_SAVE_MANAGER "Build the Go native save manager on Linux when Go is available" ON)
set(SRR2_P3D_PDDI "OpenGL" CACHE STRING "Pure3D PDDI implementation")
if(VITA)
	option(SRR2_USE_VITAGL "Use VitaGL as an OpenGL (ES) implementation" ON)
	set_property(CACHE SRR2_P3D_PDDI PROPERTY STRINGS "OpenGL" "GLES1" "GLES2" "GXM")
else()
	set_property(CACHE SRR2_P3D_PDDI PROPERTY STRINGS "OpenGL" "GLES1" "GLES2")
endif()

if(VITA)
	include("${VITASDK}/share/vita.cmake" REQUIRED)

	add_definitions(-DRAD_VITA)
	## Configuration options for this app
	# Display name (under bubble in LiveArea)
	set(VITA_APP_NAME ${SRR2_TITLE})
	# Unique ID must be exactly 9 characters. Recommended: XXXXYYYYY where X =
	# unique string of developer and Y = a unique number for this app
	set(VITA_TITLEID  "ROADRAGE2")
	# Optional version string to show in LiveArea's more info screen
	set(VITA_VERSION ${SRR2_VERSION})
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

# SDL3 support in pddi/gl/display_win32/gldisplay.cpp has API drift and does
# not compile yet, so SDL3 is opt-in and SDL2 is the default.
option(SRR2_USE_SDL3 "Prefer SDL3 over SDL2 when available" OFF)
if(NOT VITA AND SRR2_USE_SDL3)
	find_package(SDL3)
endif()
if(SDL3_FOUND)
	get_target_property(SDL_INCLUDE_DIR SDL3::Headers INTERFACE_INCLUDE_DIRECTORIES)
	include_directories(${SDL_INCLUDE_DIR}/SDL3)
	set(SDL_LIBRARY SDL3::SDL3)
else()
	find_package(SDL2 REQUIRED)
	set(SDL_LIBRARY SDL2::SDL2 SDL2::SDL2main)
endif()
find_package(PNG REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if(APPLE)
	# The system OpenAL framework is deprecated and lacks efx.h; prefer
	# Homebrew openal-soft when present.
	list(APPEND CMAKE_PREFIX_PATH /opt/homebrew/opt/openal-soft /usr/local/opt/openal-soft)
	set(CMAKE_FIND_FRAMEWORK LAST)
endif()
find_package(OpenAL REQUIRED)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-fvisibility=hidden)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_compile_options(
			-Werror
			-ggdb
		)
	endif()
	if(VITA)
		add_compile_options(
			-mtune=cortex-a9
			-mfpu=neon
		)
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DRAD_DEBUG)
else()
	add_definitions(-DRAD_RELEASE)
endif()

add_definitions(
	-DRAD_CONSOLE
	-DRAD_WIN32
	-DRAD_SDL
)

if(SRR2_REGION STREQUAL "PAL")
	add_definitions(-DPAL)
endif()
if(SRR2_USE_DLMALLOC)
	add_definitions(-DUSE_DOUG_LEA_HEAP)
endif()
if(SRR2_REROUTE_ALL_HEAP_ALLOCATIONS)
	add_definitions(-DREROUTE_ALL_HEAP_ALLOCATIONS)
endif()
if(SRR2_USE_BINK)
	add_definitions(-DRAD_MOVIEPLAYER_USE_BINK)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET
			libavformat
			libavcodec
			libavutil
			libswresample
			libswscale
	)
endif()
if(SRR2_MICROPROFILE)
	add_definitions(-DMICROPROFILE_ENABLED)
endif()
if(SRR2_P3D_CG)
	add_definitions(-DRAD_CG)
endif()
if(SRR2_USE_VITAGL)
    add_definitions(-DRAD_VITAGL)
endif()
if(SRR2_P3D_PDDI STREQUAL "GXM")
    add_definitions(-DRAD_GXM)
endif()
if(SRR2_P3D_PDDI STREQUAL "GLES1")
	add_definitions(-DRAD_GLES)
	add_definitions(-DRAD_GLES_VERSION=1)
endif()
if(SRR2_P3D_PDDI STREQUAL "GLES2")
	add_definitions(-DRAD_GLES)
	add_definitions(-DRAD_GLES_VERSION=2)
endif()

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(libs)
add_subdirectory(code)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	if(SRR2_BUILD_SAVE_MANAGER)
		find_program(SRR2_GO_EXECUTABLE go)
		if(SRR2_GO_EXECUTABLE)
			execute_process(
				COMMAND "${SRR2_GO_EXECUTABLE}" version
				OUTPUT_VARIABLE SRR2_GO_VERSION_OUTPUT
				OUTPUT_STRIP_TRAILING_WHITESPACE
				ERROR_QUIET
			)
			string(REGEX MATCH "go([0-9]+\\.[0-9]+\\.[0-9]+)" SRR2_GO_VERSION_MATCH "${SRR2_GO_VERSION_OUTPUT}")
			set(SRR2_GO_VERSION "${CMAKE_MATCH_1}")
			if(SRR2_GO_VERSION AND NOT SRR2_GO_VERSION VERSION_LESS "1.26.5")
				file(GLOB_RECURSE SRR2_SAVE_MANAGER_SOURCES CONFIGURE_DEPENDS
					"${CMAKE_SOURCE_DIR}/tools/savegame/*.go"
				)
				set(SRR2_SAVE_MANAGER_BINARY "${CMAKE_CURRENT_BINARY_DIR}/srr2-save")
				add_custom_command(
					OUTPUT "${SRR2_SAVE_MANAGER_BINARY}"
					COMMAND "${SRR2_GO_EXECUTABLE}" build -trimpath -o "${SRR2_SAVE_MANAGER_BINARY}" ./cmd/srr2-save
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tools/savegame"
					DEPENDS ${SRR2_SAVE_MANAGER_SOURCES}
						"${CMAKE_SOURCE_DIR}/tools/savegame/go.mod"
						"${CMAKE_SOURCE_DIR}/tools/savegame/go.sum"
					COMMENT "Building srr2-save"
					VERBATIM
				)
				add_custom_target(srr2-save-tool ALL DEPENDS "${SRR2_SAVE_MANAGER_BINARY}")
				install(PROGRAMS "${SRR2_SAVE_MANAGER_BINARY}"
					DESTINATION "${CMAKE_INSTALL_BINDIR}"
				)
			else()
				message(STATUS "${SRR2_GO_VERSION_OUTPUT} is too old or unrecognized; srr2-save requires Go 1.26.5+")
			endif()
		else()
			message(STATUS "Go was not found; set SRR2_BUILD_SAVE_MANAGER=OFF or install Go 1.26.5+ to build srr2-save")
		endif()
	endif()

	install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	)
	install(PROGRAMS
		"${CMAKE_SOURCE_DIR}/tools/linux-runtime/srr2-launch"
		DESTINATION "${CMAKE_INSTALL_BINDIR}"
		RENAME srr2
	)
	# Keep the validator next to the launcher so both installed and source-tree
	# launch paths use exactly the same preflight checks.
	install(PROGRAMS
		"${CMAKE_SOURCE_DIR}/tools/linux-runtime/validate-assets.sh"
		DESTINATION "${CMAKE_INSTALL_BINDIR}"
		RENAME srr2-validate-assets
	)
	install(FILES
		"${CMAKE_SOURCE_DIR}/tools/linux-runtime/srr2.desktop"
		DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
	)
	install(FILES
		"${CMAKE_SOURCE_DIR}/sce_sys/icon0.png"
		DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps"
		RENAME srr2.png
	)
	install(FILES
		"${CMAKE_SOURCE_DIR}/README.md"
		DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}"
	)
	# Preserve the source-tree relative documentation links after installation.
	install(FILES
		"${CMAKE_SOURCE_DIR}/port-docs/linux-native-port.md"
		"${CMAKE_SOURCE_DIR}/port-docs/savegame-tool.md"
		DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}/port-docs"
	)
	install(FILES
		"${CMAKE_SOURCE_DIR}/tools/savegame/README.md"
		DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}/tools/savegame"
	)
	install(FILES
		"${CMAKE_SOURCE_DIR}/docs/sdl-keyboard-mouse-controls.md"
		DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${PROJECT_NAME}/docs"
	)
endif()

if(VITA)
	## Create Vita files
	if(SRR2_P3D_PDDI STREQUAL "GLES2" AND NOT SRR2_USE_VITAGL)
		vita_create_self(${PROJECT_NAME}.self code/${PROJECT_NAME} UNSAFE)
		# The FILE directive lets you add additional files to the VPK, the syntax is
		# FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
		vita_create_vpk(${SRR2_FILENAME}${SRR2_REGION}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
			VERSION ${VITA_VERSION}
			NAME ${VITA_APP_NAME}
			FILE sce_sys/icon0.png sce_sys/icon0.png
			sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
			sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
			sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
			art art

			#suprx
			module/libGLESv2.suprx module/libGLESv2.suprx
			module/libgpu_es4_ext.suprx module/libgpu_es4_ext.suprx
			module/libIMGEGL.suprx module/libIMGEGL.suprx
			module/libpvrPSP2_WSEGL.suprx module/libpvrPSP2_WSEGL.suprx
		)
	else()
		vita_create_self(${PROJECT_NAME}.self code/${PROJECT_NAME})
		# The FILE directive lets you add additional files to the VPK, the syntax is
		# FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
		vita_create_vpk(${SRR2_FILENAME}${SRR2_REGION}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
			VERSION ${VITA_VERSION}
			NAME ${VITA_APP_NAME}
			FILE sce_sys/icon0.png sce_sys/icon0.png
			sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
			sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
			sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
			art art

			#gxp
			libs/pure3d/pddi/gxm/shaders/vertex_cg.gxp shaders/vertex_cg.gxp
			libs/pure3d/pddi/gxm/shaders/color_cg.gxp shaders/color_cg.gxp
			libs/pure3d/pddi/gxm/shaders/texture_cg.gxp shaders/texture_cg.gxp
			libs/pure3d/pddi/gxm/shaders/alpha_cg.gxp shaders/alpha_cg.gxp
		)
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
	nx_generate_nacp(OUTPUT ${PROJECT_NAME}.nacp
		NAME ${SRR2_TITLE}
		AUTHOR "Radical Entertainment"
		VERSION ${SRR2_VERSION}
	)
	nx_create_nro(code/${PROJECT_NAME}
		OUTPUT ${SRR2_FILENAME}${SRR2_REGION}.nro
		ICON ${PROJECT_NAME}.jpg
		NACP ${PROJECT_NAME}.nacp
		ROMFS romfs
	)
endif()
