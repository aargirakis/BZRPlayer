cmake_minimum_required(VERSION 3.28)

set(NAME "fluidsynth")
set(VERSION "2.5.5")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "FluidSynth (BW MIDI Sequencer)" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "v${VERSION}.tar.gz")
set(URL "https://github.com/FluidSynth/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "0827eefc06f66157c332d7bd0d65ee81be5d4c795f214db7ba0e1c70ee394430")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")

add_subdirectory(deps/gcem)

set(BW_NAME "bw-midi-sequencer")
set(BW_VERSION "0e6d9b14e6ee45afc1a9edea65a8e79d5569ec53")
string(SUBSTRING ${BW_VERSION} 0 12 BW_VERSION_SHORT)
set(BW_NAME_VERSIONED "${BW_NAME}-${BW_VERSION_SHORT}")
set(BW_FILENAME "${BW_NAME}-${BW_VERSION}.tar.gz")
set(BW_FILENAME_URL "${BW_VERSION}.tar.gz")
set(BW_URL "https://github.com/Wohlstand/${BW_NAME}/archive/${BW_FILENAME_URL}")
set(BW_SHA_256_HASH "01d33c04643f702f1d32f8b103401770fd2b5e94c82bb85da0c01ac85edc5e23")
set(BW_UNPACKED_DIR "${BW_NAME}-${BW_VERSION}")
download_patch_and_add(
        ${BW_NAME_VERSIONED} ${BW_FILENAME} ${BW_URL} ${BW_SHA_256_HASH} true ${BW_UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_${BW_NAME} "${EXTERNAL_SOURCE_DIR}")

#TODO -DNO_GUI=1

#TODO Add soundfonts: roland sc55 & mt-32, sb16, awe32, virtualdreams (from ${EXTERNAL_SOURCE_DIR}/src/sf2)
#TODO check https://musical-artifacts.com/artifacts/1481/Phoenix_MT-32.sf2

#[[ TODO
#include <fluid_sys.h>
#include <fluid_synth.h>
delete_fluid_player
delete_fluid_settings
delete_fluid_synth
fluid_player_add_mem
fluid_player_get_status
fluid_player_play
fluid_player_seek
fluid_player_stop
fluid_settings_setnum
fluid_sfloader_set_callbacks
fluid_synth_add_sfloader
fluid_synth_sfload
fluid_synth_write_float
new_fluid_defsfloader
new_fluid_player
new_fluid_settings
new_fluid_synth]]

#TODO HAVE_PTHREAD_H ?
#TODO HAVE_WINDOWS_H & MINGW32 & HAVE_IO_H on win only
#TODO NO_GUI ?
#TODO search all this to see how they are used
set(HAVE_ERRNO_H 1)
set(HAVE_FCNTL_H 1)
set(HAVE_INTTYPES_H 1)
set(HAVE_LIMITS_H 1)
set(HAVE_MATH_H 1)
set(HAVE_SIGNAL_H 1)
set(HAVE_STDARG_H 1)
set(HAVE_STDINT_H 1)
set(HAVE_STDIO_H 1)
set(HAVE_STDLIB_H 1)
set(HAVE_STRINGS_H 1)
set(HAVE_STRING_H 1)
set(HAVE_SYS_STAT_H 1)
set(HAVE_SYS_TIME_H 1)
set(HAVE_SYS_TYPES_H 1)
set(HAVE_UNISTD_H 1)
set(WITH_FLOAT 1)
set(HAVE_SINF 1)
set(HAVE_COSF 1)
set(HAVE_FABSF 1)
set(HAVE_POWF 1)
set(HAVE_SQRTF 1)
set(HAVE_LOGF 1)
set(HAVE_SOCKLEN_T 1)
set(OSAL_embedded 1)
configure_file(
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/config.cmake
        ${EXTERNAL_SOURCE_DIR_${NAME}}/config.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/bindings/filerender.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/bindings/fluid_cmd.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/bindings/fluid_filerenderer.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/drivers/fluid_adriver.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/drivers/fluid_mdriver.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/gentables/fluid_cb2amp.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/gentables/fluid_ct2hz.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/gentables/fluid_pan.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/gentables/fluid_concave.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/gentables/fluid_convex.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/gentables/fluid_interp_coeff.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/gentables/fluid_interp_coeff_linear.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/gentables/fluid_interp_coeff_sinc7.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/midi/fluid_midi.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/midi/fluid_midi_router.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_adsr_env.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_chorus.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_iir_filter.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_iir_filter_impl.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_lfo.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_rev.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_rvoice.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_rvoice_dsp.cpp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_rvoice_event.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice/fluid_rvoice_mixer.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/sfloader/fluid_defsfont.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/sfloader/fluid_samplecache.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/sfloader/fluid_sffile.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/sfloader/fluid_sfont.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/synth/fluid_chan.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/synth/fluid_gen.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/synth/fluid_mod.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/synth/fluid_synth.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/synth/fluid_synth_monopoly.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/synth/fluid_tuning.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/synth/fluid_voice.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/utils/fluid_conv.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/utils/fluid_hash.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/utils/fluid_list.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/utils/fluid_ringbuffer.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/utils/fluid_settings.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/utils/fluid_sys.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/utils/fluid_sys_embedded.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_${NAME}}
        ${EXTERNAL_SOURCE_DIR_${NAME}}/include
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/bindings
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/drivers
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/midi
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/rvoice
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/sfloader
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/synth
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/utils

        ${EXTERNAL_SOURCE_DIR_gcem}/include

        ${EXTERNAL_SOURCE_DIR_${BW_NAME}}/include

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

set(FLUIDSYNTH_DATA_DIR "${PLUGINS_DIR}/fluidsynth")
set(GM_SF2 "gm.sf2")
target_compile_definitions(${PROJECT_NAME} PRIVATE OSAL_embedded=1) #TODO why not using config.h one?
target_compile_definitions(${PROJECT_NAME} PRIVATE GM_SF2_PATH="${FLUIDSYNTH_DATA_DIR}/${GM_SF2}")

target_link_libraries(${PROJECT_NAME} PRIVATE ${MATH_LIBRARY})

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)

cmake_path(SET FLUIDSYNTH_DATA_PATH NORMALIZE ${OUTPUT_DIR}/${DATA_DIR}/${FLUIDSYNTH_DATA_DIR})

add_custom_target(copy-lib-${NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/dist/${GM_SF2} ${FLUIDSYNTH_DATA_PATH}
        VERBATIM)