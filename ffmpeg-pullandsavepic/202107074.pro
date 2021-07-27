TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp
#指定生成exe的目录------------------
UI_DIR=obj/GUI
MOC_DIR=obj/Moc
OBJECTS_DIR=obj/Obj
#将输出文件直接放到源码目录下的bin目录下，将dll都放在了次目录中，用以解决运行后找不到dll的问#DESTDIR=$$PWD/bin/
contains(QT_ARCH, i386) {
message("32-bit")
DESTDIR = $${PWD}/bin32
} else {
message("64-bit")
DESTDIR = $${PWD}/bin64
}

#---------------------------------

SOURCES +=
#学习时添加的64位现在路径已变
#INCLUDEPATH += $$PWD/lib/ffmpeg/include
#//LIBS += -L$$PWD/lib/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale



win32{

contains(QT_ARCH, i386) {
message("32-bit")
INCLUDEPATH += $$PWD/lib/win32/ffmpeg/include
$$PWD/src

LIBS += -L$$PWD/lib/win32/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

} else {
message("64-bit")
INCLUDEPATH += $$PWD/lib/win64/ffmpeg/include
$$PWD/src

LIBS += -L$$PWD/lib/win64/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
}

}
