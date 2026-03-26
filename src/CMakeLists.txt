set(QT_REQUIRED_LIBS Core Gui Widgets Network Concurrent)
set(REQUIRED_LIBS)
set(REQUIRED_LIBS_QUALIFIED Qt::Core Qt::Gui Qt::Widgets Qt::Network Qt::Concurrent)
set(REQUIRED_WIN_LIBS gcc_s_seh-1 stdc++-6 winpthread-1)

aux_source_directory(. SOURCE_SRC)

file(GLOB HEAD_FILES ${CMAKE_SOURCE_DIR}/include/*.h)
add_executable(${PROJECT_NAME} ${SOURCE_SRC} ${HEAD_FILES} ${CMAKE_BINARY_DIR}/resource.rc ${CMAKE_SOURCE_DIR}/resources.qrc)

find_package(Qt${QT_VERSION} COMPONENTS ${QT_REQUIRED_LIBS} REQUIRED)
foreach (lib ${REQUIRED_LIBS})
    find_package(${lib} REQUIRED)
endforeach ()
target_link_libraries(${PROJECT_NAME} PUBLIC ${REQUIRED_LIBS_QUALIFIED} ${REQUIRED_LIBS} imagequant)

if (CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE ON)
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-ignored-attributes)

#add_custom_command(TARGET ${PROJECT_NAME}
#        POST_BUILD
#        # COMMAND_EXPAND_LISTS
#        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
#        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/assets"
#        COMMENT "copy assets ...")
add_custom_command(TARGET ${PROJECT_NAME}
        PRE_BUILD
        # COMMAND_EXPAND_LISTS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/3rd/oxipng/oxipng.exe" "${EXECUTABLE_OUTPUT_PATH}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:imagequant> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "copy dll and exe ...")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E make_directory
#            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats"
#    )
#
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            "${CMAKE_PREFIX_PATH}/plugins/imageformats/qwebp.dll"
#            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/"
#    )
else ()
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
    qt_generate_deploy_app_script(TARGET ${PROJECT_NAME}
            OUTPUT_SCRIPT deploy_script
            NO_TRANSLATIONS
            NO_COMPILER_RUNTIME
            NO_UNSUPPORTED_PLATFORM_ERROR
            DEPLOY_TOOL_OPTIONS "--release
                                --skip-plugin-types iconengines,generic,styles
                                --no-opengl-sw
                                --no-system-d3d-compiler
                                --plugindir ${CMAKE_INSTALL_PREFIX}/bin/plugins")
    install(SCRIPT ${deploy_script} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
    install(CODE "file (REMOVE \"${CMAKE_INSTALL_PREFIX}/bin/qt.conf\")")

    install(FILES "${CMAKE_SOURCE_DIR}/3rd/oxipng/oxipng.exe" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
    install(FILES $<TARGET_FILE:imagequant> DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
    # 可选：如果需要排除某些文件/目录，可以添加 EXCLUDE 模式
    # PATTERN "*.tmp" EXCLUDE
    # PATTERN "build" EXCLUDE
    add_custom_target(generateInstaller COMMAND
            ${CMAKE_COMMAND}
            -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DPROJECT_NAME=${PROJECT_NAME}
            -DBINARY_CREATOR_PATH=${BINARY_CREATOR_PATH}
            -P ${CMAKE_SOURCE_DIR}/cmake/installer-generator.cmake)
endif ()