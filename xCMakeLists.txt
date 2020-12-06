cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set (CMAKE_DEBUG_POSTFIX "")
set (BUILD_SHARED_LIBS TRUE)
set (VarroaPopPCH "${CMAKE_SOURCE_DIR}/stdafx.h")

include("HunterGate.cmake")

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.235.tar.gz"
    SHA1 "d8992c8a66c1ff2f68566c658ddaa465eca844e4"
)

project(libportcode)


#Add the fmt library to get access to string formatting.  part of vcpkg
find_package(fmt CONFIG REQUIRED)
HUNTER_ADD_PACKAGE(fmt)

#Boost frameword
HUNTER_ADD_PACKAGE(Boost COMPONENTS system filesystem)

include_directories(CMAKE_INCLUDE_DIRECTORIES_BEFORE .)
include_directories(${PROJECT_BINARY_DIR})

find_package(Boost CONFIG REQUIRED system filesystem)

set(SOURCES carchive.cpp
            carray.cpp
            ccmdtarget.cpp
            cdialog.cpp
            cfile.cpp
            cmapstringtoob.cpp
            cobject.cpp
            coblist.cpp
            coledatetime.cpp
            cptrlist.cpp
            cstring.cpp
            cstringarray.cpp
            ctime.cpp
            cuintarray.cpp
            position.cpp
            stdafx.cpp
            varroapop.cpp )

include(GenerateExportHeader)

add_library(libportcode SHARED ${SOURCES} ".gitignore.txt")

#generate_export_header(libportcode)

GENERATE_EXPORT_HEADER( libportcode
                 BASE_NAME libportcode
                 EXPORT_MACRO_NAME libportcode_EXPORT
                 EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/libportcode_Export.h
                 #STATIC_DEFINE MyLibrary_BUILT_AS_STATIC
            )

target_include_directories(libportcode PUBLIC  ${PROJECT_SOURCE_DIR})

target_link_libraries(libportcode PRIVATE fmt::fmt)
target_link_libraries(libportcode PRIVATE Boost::system Boost::filesystem)
target_precompile_headers(libportcode PRIVATE ${VarroaPopPCH})
