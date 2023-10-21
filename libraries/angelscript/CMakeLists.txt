cmake_minimum_required(VERSION 3.19.1)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sdk/angelscript/projects/cmake/)

set(ANGEL_ADD_ON_SOURCE 
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/datetime/datetime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/debugger/debugger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptany/scriptany.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptarray/scriptarray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptbuilder/scriptbuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptdictionary/scriptdictionary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptfile/scriptfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptfile/scriptfilesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptgrid/scriptgrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scripthandle/scripthandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scripthelper/scripthelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptmath/scriptmath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptmath/scriptmathcomplex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptstdstring/scriptstdstring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/scriptstdstring/scriptstdstring_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/serializer/serializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on/weakref/weakref.cpp
)

add_library(angelscript_addon STATIC ${ANGEL_ADD_ON_SOURCE})

set(ANGEL_INCLUDE_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/angelscript/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk/add_on
)

# Link the debug and release libraries to the project
target_link_libraries(angelscript_addon PUBLIC 
    angelscript
)

target_include_directories(angelscript_addon PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ANGEL_INCLUDE_DIR}
)