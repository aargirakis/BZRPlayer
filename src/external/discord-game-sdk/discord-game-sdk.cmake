cmake_minimum_required(VERSION 3.28)

set(NAME "discord-game-sdk")
set(VERSION "3.2.1")
set(NAME_VERSIONED "${NAME}-${VERSION}")
string(REPLACE "-" "_" NAME2 "${NAME}")
set(FILENAME "${NAME2}.zip")
set(URL "https://dl-game-sdk.discordapp.net/${VERSION}/${FILENAME}")
set(SHA_256_HASH "6757bb4a1f5b42aa7b6707cbf2158420278760ac5d80d40ca708bb01d20ae6b4")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ""
        ${CMAKE_CURRENT_LIST_DIR}/patches)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")

#TODO linux only (temporary):
#file(MAKE_DIRECTORY ${OUTPUT_PATH}/${LIB_DIR})

add_custom_target(copy-lib-${NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_${NAME}}/lib/x86_64/discord_game_sdk.dll
        ${OUTPUT_PATH}
        #TODO linux only:
        #${OUTPUT_PATH}/${LIB_DIR}
        VERBATIM)
