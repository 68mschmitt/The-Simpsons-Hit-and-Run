cmake_minimum_required(VERSION 3.16)

project(SRR2LinuxPoc LANGUAGES CXX)

option(SRR2_BUILD_LINUX_POC "Build the Linux-native proof-of-concept executable" ON)

if(SRR2_BUILD_LINUX_POC)
    include(cmake/linux-poc.cmake)
endif()
