TARGET = tracker
include(tracker.pri) 
include(../plugins.inc) 

greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
