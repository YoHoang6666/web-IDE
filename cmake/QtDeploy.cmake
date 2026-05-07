function(webide_deploy_qt target)
  if(NOT TARGET ${target})
    return()
  endif()

  if(WIN32)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Run windeployqt for ${target} during packaging"
      VERBATIM
    )
  endif()
endfunction()
