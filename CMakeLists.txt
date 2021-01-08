cmake_minimum_required(VERSION 3.12)

#file(STRINGS cmake.properties ConfigContents)
foreach(NameAndValue ${ConfigContents})
    # Strip leading spaces
    string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
    # Find variable name
    string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
    # Find the value
    string(REPLACE "${Name}=" "" Value ${NameAndValue})
    # Set the variable
    set(${Name} "${Value}")
endforeach()

set(LIB_NAME sergut)
project(${LIB_NAME} VERSION 1.2.0)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Enable or disable tests
option(BUILD_TESTS "Enables tests building" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(QT_MIN_REQUIRED 5.12)
find_package(Qt5 ${QT_MIN_REQUIRED} CONFIG REQUIRED COMPONENTS
        Core
        )

include_directories(lib)

#add_subdirectory(dev-tools)
#add_subdirectory(src)

add_library(${LIB_NAME}
    lib/rapidjson/document.h
    lib/sergut/DeserializerBase.h
    lib/sergut/JsonDeserializerBase.h
    lib/sergut/JsonDeserializerQt.cpp
    lib/sergut/JsonDeserializerQt.h
    lib/sergut/JsonSerializerBase.cpp
    lib/sergut/JsonSerializerBase.h
    lib/sergut/JsonSerializerQt.h
    lib/sergut/Misc.h
    lib/sergut/misc/ReadHelper.h
    lib/sergut/ParsingException.h
    lib/sergut/SerializationException.h
    lib/sergut/SerializerBase.h
    lib/sergut/TypeTraits.h
    lib/sergut/Util.h
        )

target_link_libraries(${LIB_NAME} PUBLIC
        Qt5::Core
        )

# ************** test ***************
if (BUILD_TESTS)

    #    enable_testing()
    #    add_subdirectory(tests)

endif (BUILD_TESTS)
