include(../../make/config.inc)

TARGET     = otr
TEMPLATE   = lib
CONFIG    -= qt
CONFIG    += staticlib warn_off
DESTDIR    = ../../libs

LIBS += -lgpg-error -lgcrypt

LIBGPG_ERROR_DIR = $$PWD/../gpg-error/SMP/src
LIBGCRYPT_DIR = $$PWD/../gcrypt/SMP
INCLUDEPATH += src $$LIBGPG_ERROR_DIR $$LIBGCRYPT_DIR

include(otr.pri)
