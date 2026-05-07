option(WEBIDE_ENABLE_SANITIZERS "Enable developer sanitizers" OFF)

function(webide_enable_sanitizers target)
  if(NOT WEBIDE_ENABLE_SANITIZERS)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PRIVATE -fsanitize=address,undefined)
    target_link_options(${target} PRIVATE -fsanitize=address,undefined)
  endif()
endfunction()
