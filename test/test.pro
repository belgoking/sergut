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
    Debug.cpp \
    TestVersionTracker.cpp \
    hypercall/hypercall.cpp \
    main.cpp \
    sergut/TestJavaClassGenerator.cpp \
    sergut/TestSergut.cpp \
    sergut/XmlDeSerializerTiny.cpp \
    sergut/marshaller/TestRequestClient.cpp \
    sergut/marshaller/TestRequestServer.cpp \
    sergut/unicode/TestUtf16Codec.cpp \
    sergut/unicode/TestUtf8Codec.cpp \
    sergut/xml/TestPullParser.cpp \
    sergut/xml/TestTextDecodingHelper.cpp \


HEADERS += \
    sergut/TestSupportClasses.h \
    sergut/marshaller/TestSupportClassesMarshaller.h \
