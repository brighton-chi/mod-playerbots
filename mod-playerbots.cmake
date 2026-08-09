#
# Auto-included by modules/CMakeLists.txt (via the per-module `include(... OPTIONAL)`
# hook) after the `modules` target has been created. Because that is an include()
# and not an add_subdirectory(), this runs in the same directory scope as the
# target definition, so source file properties set here take effect.
#
# AzerothCore's msvc/settings.cmake strips CMake's default /W flag from
# CMAKE_CXX_FLAGS and never restores a level, so cl.exe falls through to /W1.
# The core then remaps the unused-symbol warnings to level 3 (/w34100 /w34101
# /w34189), which means they can never fire. Re-levelling them to 1 does:
#
#   C4100 - unreferenced formal parameter
#   C4101 - unreferenced local variable
#   C4189 - local variable is initialized but not referenced
#   C4505 - unreferenced local function has been removed
#
# MODULES is "static", so mod-playerbots is compiled into the shared `modules`
# target alongside every other static module. Setting the option on the target
# would raise the level for all of them, so scope it to this module's sources.

if(MSVC)
  GetPathToModuleSource(mod-playerbots PLAYERBOTS_WARNING_SCOPE_PATH)

  unset(PLAYERBOTS_WARNING_SCOPE_SOURCES)
  CollectSourceFiles(${PLAYERBOTS_WARNING_SCOPE_PATH} PLAYERBOTS_WARNING_SCOPE_SOURCES)

  # Rather than raising the global level with /W3 - which also pulls in every
  # other level-2/3 warning, notably the C4244/C4267 conversion noise - assign
  # just the wanted warnings to level 1 so they fire under the default /W1.
  # Nothing else changes, so no suppression list is needed.
  set_source_files_properties(${PLAYERBOTS_WARNING_SCOPE_SOURCES}
    PROPERTIES
      COMPILE_OPTIONS "/w14100;/w14101;/w14189;/w14505")
endif()
