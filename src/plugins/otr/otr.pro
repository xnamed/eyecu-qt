TARGET = otr
LIBS += -L../../libs
LIBS += -lotr -lgcrypt -lgpg-error

INCLUDEPATH += ../../thirdparty/otr/src

include(otr.pri)
include(../plugins.inc)
