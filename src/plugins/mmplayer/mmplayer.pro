TARGET = mmplayer 

include(mmplayer.pri)
include(../plugins.inc)

contains(QT_CONFIG, multimediakit): {
	CONFIG += mobility
	MOBILITY += multimedia
}
else: contains(QT_CONFIG, multimedia): QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4){
	QT += qpffmpeg qputil
	CONFIG += c++11
} else {
	CONFIG += qpffmpeg qputil
	win32-g++:QMAKE_CXXFLAGS += -std=c++11
}
