#
# Auto-included by modules/CMakeLists.txt after the `modules` target is created.
#
# AzerothCore's msvc/settings.cmake strips CMake's default /W flag from
# CMAKE_CXX_FLAGS and never puts a level back, so cl.exe falls through to /W1.
# The core then remaps the unused-symbol warnings to level 3 (/w34100 /w34101
# /w34189 /w34389), which means they can never fire. Setting the level here
# brings them back:
#
#   C4100 - unreferenced formal parameter
#   C4101 - unreferenced local variable
#   C4189 - local variable is initialized but not referenced
#   C4389 - signed/unsigned mismatch in an equality comparison
#
# MODULES is "static", so mod-playerbots compiles into the `modules` target;
# this therefore applies to every statically-linked module, not just this one.

if(MSVC)
  target_compile_options(modules PRIVATE /W3)

  # For /W4 instead, third-party headers (boost, g3d, mysql) pulled into module
  # translation units get scanned at the same level, so silence them:
  #   target_compile_options(modules PRIVATE /W4 /external:anglebrackets /external:W0)
endif()
