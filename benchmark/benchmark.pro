include(../local.pri)

TEMPLATE = app

QT -= core
QT -= gui

TARGET = benchmark

CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

LIBS += -L "$${OUT_PWD}/../lib" -L "$${CPP_TINYXML_LIB_PATH}" -lsergut -ltinyxml2

QMAKE_CXXFLAGS += -ggdb3

INCLUDEPATH = ../lib "$${CPP_TINYXML_INCLUDE_PATH}"

SOURCES += \
    main.cpp

HEADERS +=

