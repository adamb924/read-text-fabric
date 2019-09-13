#-------------------------------------------------
#
# Project created by QtCreator 2018-10-31T16:49:41
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TARGET = read-text-fabric
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


HEADERS += databaseadapter.h \
    reader.h \
    tffile.h
SOURCES += main.cpp databaseadapter.cpp \
    reader.cpp \
    tffile.cpp
