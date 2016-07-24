include(../local.pri)

TEMPLATE = app

QT -= core
QT -= gui

TARGET = test

CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

LIBS += -L "$${OUT_PWD}/../lib" -L "$${CPP_TINYXML_LIB_PATH}" -lsergut -ltinyxml2 -ltinyxml

QMAKE_CXXFLAGS += -ggdb3

INCLUDEPATH = ../lib "$${CPP_CATCH_INCLUDE_PATH}" "$${CPP_TINYXML_INCLUDE_PATH}"

SOURCES += \
    main.cpp \
    sergut/TestSergut.cpp \
    xml/TestPullParser.cpp \
    xml/TestTextDecodingHelper.cpp \
    unicode/TestUtf8Codec.cpp \
    unicode/TestUtf16Codec.cpp \
    sergut/XmlDeSerializerTiny.cpp

HEADERS +=

