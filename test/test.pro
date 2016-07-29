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
    hypercall/hypercall.cpp \
    main.cpp \
    sergut/TestJavaClassGenerator.cpp \
    sergut/TestSergut.cpp \
    sergut/XmlDeSerializerTiny.cpp \
    unicode/TestUtf16Codec.cpp \
    unicode/TestUtf8Codec.cpp \
    xml/TestPullParser.cpp \
    xml/TestTextDecodingHelper.cpp \

HEADERS +=
    sergut/TestSupportClasses.h \

