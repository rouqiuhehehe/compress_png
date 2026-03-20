file(MAKE_DIRECTORY ${INSTALL_PREFIX}/installer/config
        ${INSTALL_PREFIX}/installer/packages/${PROJECT_NAME}/data
        ${INSTALL_PREFIX}/installer/packages/${PROJECT_NAME}/meta)
file(COPY ${INSTALL_PREFIX}/bin/ DESTINATION ${CMAKE_INSTALL_PREFIX}/installer/packages/${PROJECT_NAME}/data)
configure_file(${SOURCE_DIR}/config/install-config.xml.in
        ${INSTALL_PREFIX}/installer/config/config.xml @ONLY)
configure_file(${SOURCE_DIR}/config/install-package.xml.in
        ${INSTALL_PREFIX}/installer/packages/${PROJECT_NAME}/meta/package.xml @ONLY)
file(COPY_FILE ${SOURCE_DIR}/config/installscript.qs
        ${INSTALL_PREFIX}/installer/packages/${PROJECT_NAME}/meta/installscript.qs)
execute_process(
        COMMAND ${BINARY_CREATOR_PATH} --offline-only -c config/config.xml -p packages ${PROJECT_NAME}Installer
        WORKING_DIRECTORY ${INSTALL_PREFIX}/installer
)

file(REMOVE_RECURSE ${INSTALL_PREFIX}/installer/config ${INSTALL_PREFIX}/installer/packages)