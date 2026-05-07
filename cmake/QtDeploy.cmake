function(webide_deploy_qt TARGET_NAME)

if(WIN32)
    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND windeployqt
                $<TARGET_FILE:${TARGET_NAME}>
    )
endif()

endfunction()