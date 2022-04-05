# CMakeList.txt : CMake project for libvpop, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable. 

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set (CMAKE_DEBUG_POSTFIX "")
set (BUILD_SHARED_LIBS TRUE)
#set (CMAKE_CXX_FLAGS "-fPIC")

project ("libvpop")


include_directories(CMAKE_INCLUDE_DIRECTORIES_BEFORE .)
include_directories(${PROJECT_BINARY_DIR})

set (SOURCES    
                stdafx.cpp
                adult.cpp
                bee.cpp
                brood.cpp
                coldstoragesimulator.cpp
                colony.cpp
                colonyresource.cpp
                daterangevalues.cpp
                egg.cpp
                epadata.cpp
                globaloptions.cpp
                ieditem.cpp
                larva.cpp
                mite.cpp
                mitetreatmentitem.cpp
                mitetreatments.cpp
                nutrientcontaminationtable.cpp
                queen.cpp
                session.cpp
                spores.cpp
                vpoplib.cpp
                weatherevents.cpp)

set (PORT_SOURCES  
                position.cpp
                cuintarray.cpp
                cstringarray.cpp
                cstring.cpp
                cptrlist.cpp
                coledatetime.cpp
                coblist.cpp
                cobject.cpp
                cmapstringtoob.cpp
                ccmdtarget.cpp
                carray.cpp)


include(GenerateExportHeader)

add_library(libvpop SHARED ${SOURCES} ${PORT_SOURCES})


GENERATE_EXPORT_HEADER( libvpop
                 BASE_NAME libvpop
                 EXPORT_MACRO_NAME libvpop_EXPORT
                 EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/libvpop_Export.h
            )

target_include_directories(libvpop PUBLIC  ${PROJECT_SOURCE_DIR})
