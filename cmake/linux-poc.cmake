option(SRR2_LINUX_POC_WITH_SDL "Enable the SDL2-backed Linux PoC platform shell when SDL2 is available" ON)

add_executable(srr2-linux-poc
    game/code/main/linuxmain.cpp
    game/code/main/linuxplatform.cpp
    game/code/main/singletons_linux_poc.cpp
    game/code/main/commandlineoptions.cpp
    game/code/main/game.cpp
    game/code/gameflow/gameflow.cpp
    game/code/contexts/context.cpp
    game/code/contexts/linux_poc_contexts.cpp
    game/code/events/eventlistener.cpp
    game/code/port/linux_poc_filesystem.cpp
    game/code/port/linux_poc_inputmanager.cpp
    game/code/port/linux_poc_loadingmanager.cpp
    game/code/port/linux_poc_memory.cpp
    game/code/port/linux_poc_renderflow.cpp
    game/code/port/linux_poc_soundmanager.cpp
)

target_compile_features(srr2-linux-poc PRIVATE cxx_std_17)

target_compile_definitions(srr2-linux-poc PRIVATE
    RAD_LINUX
    RAD_PC
    _DEBUG
    LINUX_POC
)

target_include_directories(srr2-linux-poc PRIVATE
    ${CMAKE_SOURCE_DIR}/game/code/port/stubs
    ${CMAKE_SOURCE_DIR}/game/code
)

if(SRR2_LINUX_POC_WITH_SDL)
    find_package(SDL2 QUIET)

    if(TARGET SDL2::SDL2)
        target_link_libraries(srr2-linux-poc PRIVATE SDL2::SDL2)
        target_compile_definitions(srr2-linux-poc PRIVATE LINUX_POC_WITH_SDL)
        message(STATUS "srr2-linux-poc: SDL2 platform shell enabled via SDL2::SDL2")
    elseif(SDL2_FOUND)
        target_include_directories(srr2-linux-poc PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(srr2-linux-poc PRIVATE ${SDL2_LIBRARIES})
        target_compile_definitions(srr2-linux-poc PRIVATE LINUX_POC_WITH_SDL)
        message(STATUS "srr2-linux-poc: SDL2 platform shell enabled via FindSDL2 variables")
    else()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(PC_SDL2 QUIET IMPORTED_TARGET sdl2)
        endif()

        if(TARGET PkgConfig::PC_SDL2)
            target_link_libraries(srr2-linux-poc PRIVATE PkgConfig::PC_SDL2)
            target_compile_definitions(srr2-linux-poc PRIVATE LINUX_POC_WITH_SDL)
            message(STATUS "srr2-linux-poc: SDL2 platform shell enabled via pkg-config")
        else()
            message(WARNING "srr2-linux-poc: SDL2 was requested but not found; building headless PoC shell")
        endif()
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(srr2-linux-poc PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
    )
endif()
