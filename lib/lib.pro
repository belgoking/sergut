#-------------------------------------------------
#
# Project created by QtCreator 2016-02-24T06:21:30
#
#-------------------------------------------------
include(../local.pri)

TEMPLATE = lib

QT -= core
QT -= gui

TARGET =  sergut
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11
CONFIG += staticlib

INCLUDEPATH += "$${CPP_TINYXML_INCLUDE_PATH}"

QMAKE_CXXFLAGS += -ggdb3

SOURCES += \
    sergut/JsonSerializer.cpp \
    sergut/MemberDeserializer.cpp \
    sergut/ParsingException.cpp \
    sergut/XmlDeserializer.cpp \
    sergut/XmlDeserializerTiny.cpp \
    sergut/XmlSerializer.cpp \
    unicode/Utf8Codec.cpp \
    xml/PullParser.cpp \

HEADERS += \
    misc/StringRef.h \
    sergut/DeserializerBase.h \
    sergut/Exception.h \
    sergut/JsonSerializer.h \
    sergut/MemberDeserializer.h \
    sergut/Misc.h \
    sergut/ParsingException.h \
    sergut/SerializerBase.h \
    sergut/Util.h \
    sergut/XmlDeserializer.h \
    sergut/XmlDeserializerTiny.h \
    sergut/XmlSerializer.h \
    unicode/ParseResult.h \
    unicode/Utf16Codec.h \
    unicode/Utf32Char.h \
    unicode/Utf8Codec.h \
    xml/ParseTokenType.h \
    xml/PullParser.h \
    xml/detail/BasicPullParserNV.h \
    xml/detail/Helper.h \
    xml/detail/PullParserNVUtf16BE.h \
    xml/detail/PullParserNVUtf16LE.h \
    xml/detail/PullParserNVUtf8.h \
    xml/detail/ReaderState.h \
    xml/detail/ReaderStateResetter.h \
    xml/detail/TextDecodingHelper.h \

