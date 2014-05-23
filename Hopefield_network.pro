#-------------------------------------------------
#
# Project created by QtCreator 2014-05-01T00:03:47
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = Hopefield_network
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    network.cpp \
    temperaturemodule.cpp \
    problems.cpp

HEADERS += \
    network.h \
    temperaturemodule.h \
    problems.h
