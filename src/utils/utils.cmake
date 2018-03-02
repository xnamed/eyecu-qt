file(GLOB SOURCES "*.cpp")

set(HEADERS "action.h"
            "filestorage.h"
            "iconstorage.h"
            "menu.h"
            "menubarchanger.h"
            "options.h"
            "shortcuts.h"
            "statusbarchanger.h"
            "systemmanager.h"
            "toolbarchanger.h"
            "animatedtextbrowser.h"
            "closebutton.h"
            "searchlineedit.h"
            "imagemanager.h"
            "advanceditem.h"
            "advanceditemmodel.h"
            "advanceditemdelegate.h"
            "filecookiejar.h"
            "boxwidget.h"
            "splitterwidget.h"
            "logger.h"
            "pluginhelper.h"
            "passworddialog.h")

qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
