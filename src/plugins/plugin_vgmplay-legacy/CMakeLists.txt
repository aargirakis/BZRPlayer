cmake_minimum_required(VERSION 3.28)

set(NAME "vgmplay-legacy")
set(VERSION "0.40.9.1")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "VGMPlay" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.zip")
set(FILENAME_URL "${VERSION}.zip")
set(URL "https://github.com/vgmrips/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "c797f4ac00dd9b390b3323fc2d584924eca2625b09bca9c0f8ade771ff4a681d")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/ChipMapper.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/262intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/2151intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/2203intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/2413intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/2608intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/2610intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/2612intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/3526intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/3812intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/8950intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/adlibemu_opl2.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/adlibemu_opl3.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ay8910.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ay8910_opl.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ay_intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/c140.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/c352.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/c6280.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/c6280intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/dac_control.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/emu2149.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/emu2413.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/es5503.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/es5506.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/fm2612.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/fm.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/fmopl.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/gb.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/iremga20.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/k051649.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/k053260.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/k054539.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/multipcm.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/nes_apu.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/nes_intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/np_nes_apu.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/np_nes_dmc.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/np_nes_fds.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/okim6258.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/okim6295.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/Ootake_PSG.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/panning.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/pokey.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/pwm.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/qsound_ctr.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/qsound_intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/rf5c68.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/saa1099.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/scd_pcm.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/scsp.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/scspdsp.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/segapcm.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/sn764intf.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/sn76489.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/sn76496.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/sn76496_opl.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/upd7759.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/vsu.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ws_audio.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/x1_010.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ym2151.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ym2413.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ym2413_opl.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ym2413hd.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ym2612.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ymdeltat.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ymf262.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ymf271.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ymf278b.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips/ymz280b.c
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/VGMPlay.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/VGMPlay
        ${EXTERNAL_SOURCE_DIR}/VGMPlay/chips

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_HW_SUPPORT)
#target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_ALL_CORES) #TODO

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
