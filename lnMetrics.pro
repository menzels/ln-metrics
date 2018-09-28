TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += json/include

SOURCES += \
        main.cpp \
    digraph.cpp

HEADERS += \
    apGraph.h \
    digraph.h

# Enable C++17 manually, since CONFIG += c++17/1z doesn't work yet with MSVC
# See also QTBUG-63527
QMAKE_CXXFLAGS += /std:c++17
