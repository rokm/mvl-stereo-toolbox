######################################################################
# Automatically generated by qmake (2.01a) pon. maj 13 15:58:31 2013
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

CONFIG += debug

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
    PKGCONFIG += libelas
}

# Input
SOURCES += \
    main.cpp \
    StereoCalibration.cpp \
    StereoMethod.cpp \
    StereoMethodConstantSpaceBeliefPropagationGPU.cpp \
    StereoMethodBeliefPropagationGPU.cpp \
    StereoMethodBlockMatching.cpp \
    StereoMethodBlockMatchingGPU.cpp \
    StereoMethodELAS.cpp \
    StereoMethodSemiGlobalBlockMatching.cpp \
    StereoMethodVar.cpp \
    StereoPipeline.cpp \
    Toolbox.cpp

HEADERS += \
    StereoCalibration.h \
    StereoMethod.h \
    StereoMethodConstantSpaceBeliefPropagationGPU.h \
    StereoMethodBeliefPropagationGPU.h \
    StereoMethodBlockMatching.h \
    StereoMethodBlockMatchingGPU.h \
    StereoMethodELAS.h \
    StereoMethodSemiGlobalBlockMatching.h \
    StereoMethodVar.h \
    StereoPipeline.h \
    Toolbox.h

FORMS += \
    Toolbox.ui
