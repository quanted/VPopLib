# CMakeList.txt : CMake project for libvpop, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable. 

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set (CMAKE_DEBUG_POSTFIX "")
set (BUILD_SHARED_LIBS TRUE)
#set (PORTCODE_PATH "${PROJECT_BINARY_DIR}/libportcode.lib")


include("HunterGate.cmake")

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.235.tar.gz"
    SHA1 "d8992c8a66c1ff2f68566c658ddaa465eca844e4"
)

project ("libvpop")

set (Boost_USE_STATIC_LIBS OFF)
#Add the fmt library to get access to string formatting.  part of vcpkg
HUNTER_ADD_PACKAGE(fmt)
find_package(fmt CONFIG REQUIRED)  #cannot use this for linux.  Need to figure out how to make something work for both


#Boost framework
#find_package(Boost CONFIG REQUIRED system filesystem)
#HUNTER_ADD_PACKAGE(Boost COMPONENTS system filesystem)
#find_package(Boost CONFIG REQUIRED system )
#set(BOOST_ROOT "C:/soft/boost_1_71_0")
HUNTER_ADD_PACKAGE(Boost COMPONENTS system date_time)
find_package(Boost CONFIG REQUIRED system date_time)

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
                weatherevents.cpp
                weathergriddata.cpp)

set (PORT_SOURCES  
                position.cpp
                cuintarray.cpp
                ctime.cpp
                cstringarray.cpp
                cstring.cpp
                cptrlist.cpp
                coledatetime.cpp
                coblist.cpp
                cobject.cpp
                cmapstringtoob.cpp
                #cfile.cpp
                ccmdtarget.cpp
                carray.cpp)


include(GenerateExportHeader)

add_library(libvpop SHARED ${SOURCES} ${PORT_SOURCES})


GENERATE_EXPORT_HEADER( libvpop
                 BASE_NAME libvpop
                 EXPORT_MACRO_NAME libvpop_EXPORT
                 EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/libvpop_Export.h
                 #STATIC_DEFINE MyLibrary_BUILT_AS_STATIC
            )

target_include_directories(libvpop PUBLIC  ${PROJECT_SOURCE_DIR})

target_link_libraries(libvpop PUBLIC fmt::fmt)
target_link_libraries(libvpop PUBLIC Boost::system Boost::date_time)
#target_link_libraries(libvpop PRIVATE Boost::system Boost::filesystem)
#target_link_libraries(libvpop PRIVATE libportcode)
#target_precompile_headers(libportcode PRIVATE ${VarroaPopPCH})