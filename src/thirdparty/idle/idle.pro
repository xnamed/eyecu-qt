include(../../make/config.inc)

TARGET         = idle
TEMPLATE       = lib
CONFIG        += staticlib warn_off
DESTDIR        = ../../libs
unix:!macx:!haiku {
  DEFINES     += HAVE_XSS
  QT          += x11extras
}
include(idle.pri)
