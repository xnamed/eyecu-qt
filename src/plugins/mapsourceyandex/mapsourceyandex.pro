TARGET = mapsourceyandex
os2:TARGET_SHORT = msyandex
include(mapsourceyandex.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
