TARGET = mapsourceovi
os2:TARGET_SHORT = msovi
include(mapsourceovi.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
