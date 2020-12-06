# CMakeList.txt : CMake project for libvpop, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)


# Add source to this project's executable.

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set (CMAKE_DEBUG_POSTFIX "")
set (BUILD_SHARED_LIBS TRUE)
set (PORTCODE_PATH "${PROJECT_BINARY_DIR}/libportcode.lib")

include("HunterGate.cmake")

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.235.tar.gz"
    SHA1 "d8992c8a66c1ff2f68566c658ddaa465eca844e4"
)

project ("libvpop")


#Add the fmt library to get access to string formatting.  part of vcpkg
find_package(fmt CONFIG REQUIRED)
HUNTER_ADD_PACKAGE(fmt)

#Boost framework
HUNTER_ADD_PACKAGE(Boost COMPONENTS system filesystem)

include_directories(CMAKE_INCLUDE_DIRECTORIES_BEFORE .)
include_directories(${PROJECT_BINARY_DIR})

find_package(Boost CONFIG REQUIRED system filesystem)

set (SOURCES    
                stdafx.cpp
                Adult.cpp
                Bee.cpp
                Brood.cpp
                ColdStorageSimulator.cpp
                Colony.cpp
                ColonyResource.cpp
                DateRangeValues.cpp
                Egg.cpp
                EPAData.cpp
                GlobalOptions.cpp
                IEDItem.cpp
                Larva.cpp
                Mite.cpp
                MiteTreatmentItem.cpp
                MiteTreatments.cpp
                NutrientContaminationTable.cpp
                Queen.cpp
                Spores.cpp
                VPopLib.cpp
                WeatherEvents.cpp
                WeatherGridData.cpp)

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
                cfile.cpp
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

target_link_libraries(libvpop PRIVATE fmt::fmt)
target_link_libraries(libvpop PRIVATE Boost::system Boost::filesystem)
#target_link_libraries(libvpop PRIVATE libportcode)
#target_precompile_headers(libportcode PRIVATE ${VarroaPopPCH})