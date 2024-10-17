cmake_minimum_required(VERSION 3.28)

set(LIB_NAME "discord-game-sdk")
set(LIB_VERSION "3.2.1")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
string(REPLACE "-" "_" LIB_NAME2 "${LIB_NAME}")
set(LIB_FILENAME "${LIB_NAME2}.zip")
set(LIB_URL "https://dl-game-sdk.discordapp.net/${LIB_VERSION}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "6757bb4a1f5b42aa7b6707cbf2158420278760ac5d80d40ca708bb01d20ae6b4")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true "" ${CMAKE_CURRENT_LIST_DIR}/patches
)

set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}")

#TODO linux only (temporary):
#file(MAKE_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR})

add_custom_target(
        copy-lib-${LIB_NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_${LIB_NAME}}/lib/x86_64/discord_game_sdk.dll
        ${OUTPUT_DIR}
        #TODO linux only:
        #${OUTPUT_DIR}/${LIB_DIR}
        VERBATIM)
