set(UTIL_SRC
    util/capacity.cpp
    util/externalcommand.cpp
    util/globallog.cpp
    util/helpers.cpp
    util/htmlreport.cpp
    util/report.cpp
)

set(UTIL_LIB_HDRS
    util/libpartitionmanagerexport.h
    util/capacity.h
    util/externalcommand.h
    util/globallog.h
    util/helpers.h
    util/htmlreport.h
    util/report.h
)

add_executable(kpmcore_externalcommand util/externalcommandhelper.cpp)
target_link_libraries(kpmcore_externalcommand  KF5::Auth  KF5::I18n)
install(TARGETS kpmcore_externalcommand DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(kpmcore_externalcommand org.kde.kpmcore.externalcommand root)
kauth_install_actions(org.kde.kpmcore.externalcommand util/org.kde.kpmcore.externalcommand.actions)
