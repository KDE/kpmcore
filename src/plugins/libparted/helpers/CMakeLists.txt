add_executable(kpmcore_scan scan.cpp)
target_link_libraries(kpmcore_scan ${LIBPARTED_LIBS} KF5::Auth)
install(TARGETS kpmcore_scan DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(kpmcore_scan org.kde.kpmcore.scan root)
kauth_install_actions(org.kde.kpmcore.scan org.kde.kpmcore.scan.actions)
