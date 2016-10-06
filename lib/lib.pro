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
    VersionTracker.cpp \
    sergut/JsonSerializer.cpp \
    sergut/ParsingException.cpp \
    sergut/Version.cpp \
    sergut/XmlDeserializer.cpp \
    sergut/XmlDeserializerTiny.cpp \
    sergut/XmlDeserializerTiny2.cpp \
    sergut/XmlSerializer.cpp \
    sergut/detail/JavaClassGeneratorBase.cpp \
    sergut/detail/Member.cpp \
    sergut/detail/MemberDeserializer.cpp \
    sergut/detail/NameSpace.cpp \
    sergut/detail/TypeName.cpp \
    sergut/misc/ReadHelper.cpp \
    sergut/unicode/Utf8Codec.cpp \
    sergut/xml/PullParser.cpp \

HEADERS += \
    VersionTracker.h \
    sergut/DeserializerBase.h \
    sergut/Exception.h \
    sergut/JavaClassGenerator.h \
    sergut/JsonSerializer.h \
    sergut/Misc.h \
    sergut/ParsingException.h \
    sergut/SerializationException.h \
    sergut/SerializerBase.h \
    sergut/Util.h \
    sergut/Version.h \
    sergut/XmlDeserializer.h \
    sergut/XmlDeserializerTiny.h \
    sergut/XmlDeserializerTiny2.h \
    sergut/XmlSerializer.h \
    sergut/XmlValueType.h \
    sergut/detail/DummySerializer.h \
    sergut/detail/JavaClassGeneratorBase.h \
    sergut/detail/JavaClassGeneratorBuilder.h \
    sergut/detail/Member.h \
    sergut/detail/MemberDeserializer.h \
    sergut/detail/NameSpace.h \
    sergut/detail/Nesting.h \
    sergut/detail/TypeName.h \
    sergut/detail/XmlDeserializerDomBase.h \
    sergut/detail/XmlDeserializerHelper.h \
    sergut/marshaller/MarshallingException.h \
    sergut/marshaller/RequestServer.h \
    sergut/marshaller/UnknownFunctionException.h \
    sergut/marshaller/UnsupportedFormatException.h \
    sergut/misc/ConstStringRef.h \
    sergut/misc/ReadHelper.h \
    sergut/misc/StringRef.h \
    sergut/unicode/ParseResult.h \
    sergut/unicode/Utf16Codec.h \
    sergut/unicode/Utf32Char.h \
    sergut/unicode/Utf8Codec.h \
    sergut/xml/ParseTokenType.h \
    sergut/xml/PullParser.h \
    sergut/xml/detail/BasicPullParser.h \
    sergut/xml/detail/Helper.h \
    sergut/xml/detail/ParseStack.h \
    sergut/xml/detail/PullParserUtf16BE.h \
    sergut/xml/detail/PullParserUtf16LE.h \
    sergut/xml/detail/PullParserUtf8.h \
    sergut/xml/detail/ReaderState.h \
    sergut/xml/detail/ReaderStateResetter.h \
    sergut/xml/detail/TextDecodingHelper.h \

