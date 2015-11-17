TEMPLATE    = app
QT         += opengl

####CASA
LIBS += /usr/local/Cellar/glew/1.12.0/lib/libGLEW.1.12.0.dylib

INCLUDEPATH += /usr/local/Cellar/glew/1.12.0/include
INCLUDEPATH += /usr/local/Cellar/glm/0.9.7.1/include


####UNIVERSITAT
#LIBS += -lGLEW
#INCLUDEPATH +=  /usr/include/glm
#INCLUDEPATH += ./Model

HEADERS += MyGLWidget.h

SOURCES += main.cpp \
        MyGLWidget.cpp ./Model/model.cpp
