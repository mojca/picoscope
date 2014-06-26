cmake_minimum_required (VERSION 2.6)

project (PicoScope)
# The version number.
set (PicoScope_VERSION_MAJOR 0)
set (PicoScope_VERSION_MINOR 4)

# Project sources and libraries
include_directories ("${PROJECT_SOURCE_DIR}/include")
# PicoTech libraries (only needed on Windows; on linux these files are installed already)
# include_directories ("${PROJECT_SOURCE_DIR}/lib")
link_directories("${PROJECT_SOURCE_DIR}/lib")

add_executable(run_picoscope src/run_picoscope.cpp
                             src/args.cpp
                             src/channel.cpp
                             src/measurement.cpp
                             src/picoscope.cpp
                             src/timing.cpp
                             src/trigger.cpp)

add_executable(bin2dat util/bin2dat.cpp)
# set_target_properties(bin2dat PROPERTIES OUTPUT_NAME "bin2dat${CMAKE_EXECUTABLE_SUFFIX}")
# set_target_properties(bin2dat PROPERTIES SUFFIX "${CMAKE_EXECUTABLE_SUFFIX}")

if (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
set_target_properties(bin2dat PROPERTIES SUFFIX ".exe")
endif (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")

if (MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static-libgcc -static-libstdc++ -s")
endif (MINGW)

# not used yet
option(USE_PICOSCOPE_4000 "Use support for PicoScope 4000 Series" ON)
option(USE_PICOSCOPE_6000 "Use support for PicoScope 6000 Series" ON)

if (USE_PICOSCOPE_4000)
    # include_directories(...)
    # add_subdirectory (...)
    set (EXTRA_LIBS ${EXTRA_LIBS} -lps4000)
endif (USE_PICOSCOPE_4000)
if (USE_PICOSCOPE_4000)
    # include_directories(...)
    # add_subdirectory (...)
    set (EXTRA_LIBS ${EXTRA_LIBS} -lps6000)
endif (USE_PICOSCOPE_4000)

target_link_libraries (run_picoscope ${EXTRA_LIBS})

# CMAKE_EXECUTABLE_SUFFIX

install (TARGETS bin2dat       DESTINATION bin)
install (TARGETS run_picoscope DESTINATION bin)

# CMAKE_CXX_COMPILER=i386-mingw32-g++
