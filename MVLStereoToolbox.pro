######################################################################
# Automatically generated by qmake (2.01a) pon. maj 13 15:58:31 2013
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
}

# Input
SOURCES += \
    main.cpp \
    StereoCalibration.cpp \
    StereoMethod.cpp \
    StereoMethodBlockMatching.cpp \
    StereoPipeline.cpp

HEADERS += \
    StereoCalibration.h \
    StereoMethod.h \
    StereoMethodBlockMatching.h \
    StereoPipeline.h
