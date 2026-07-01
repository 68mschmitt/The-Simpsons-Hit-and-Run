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

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(srr2-linux-poc PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
    )
endif()
