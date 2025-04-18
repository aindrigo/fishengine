if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bin OR NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/public)
    message("FishEngine warning: Steamworks binary or public files not found. Include them if you need Steam functionality.")
    return()
endif()

set(STEAMWORKS_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(STEAMWORKS_LIB ${STEAMWORKS_BIN_DIR}/win64/steam_api64.dll)
    else()
        set(STEAMWORKS_LIB ${STEAMWORKS_BIN_DIR}/steam_api.dll)
    endif()
elseif(OSX)
    set(STEAMWORKS_LIB ${STEAMWORKS_BIN_DIR}/osx/lib_steamapi.dylib)
elseif(LINUX)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(STEAMWORKS_LIB ${STEAMWORKS_BIN_DIR}/linux64/lib_steamapi.so)
    else()
        set(STEAMWORKS_LIB ${STEAMWORKS_BIN_DIR}/linux32/lib_steamapi.so)
    endif()
endif()

add_library(steamworks SHARED IMPORTED GLOBAL)
set_target_properties(steamworks PROPERTIES
    IMPORTED_IMPLIB "${STEAMWORKS_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/public"
)