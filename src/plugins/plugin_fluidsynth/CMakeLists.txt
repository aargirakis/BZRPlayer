cmake_minimum_required(VERSION 3.28)

set(NAME "fluidsynth")
set(VERSION "2.5.4")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "FluidSynth" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "v${VERSION}.tar.gz")
set(URL "https://github.com/FluidSynth/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "72f5720328fe44e2e5c67813885f0a6b4b004d048bd2eeeb0c0064074ebff530")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

add_subdirectory(deps/gcem)

#TODO add LIBINSTPATCH for DLS support
#TODO -Denable-libinstpatch=0
#TODO -DNO_GUI=1

#TODO copy gm.dls to /usr/share/bzr-player/plugins/fluidsynth
#TODO copy dls sf2/sf3 from ${EXTERNAL_SOURCE_DIR}/src/sf2 to /usr/share/bzr-player/plugins/fluidsynth
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
#set(LIBINSTPATCH_SUPPORT 1)
set(WITH_FLOAT 1)
set(HAVE_SINF 1)
set(HAVE_COSF 1)
set(HAVE_FABSF 1)
set(HAVE_POWF 1)
set(HAVE_SQRTF 1)
set(HAVE_LOGF 1)
set(HAVE_SOCKLEN_T 1)
set(OSAL_embedded 1)
set(ENABLE_NATIVE_DLS 1)
configure_file(
        ${EXTERNAL_SOURCE_DIR}/src/config.cmake
        ${EXTERNAL_SOURCE_DIR}/config.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/bindings/filerender.c
        ${EXTERNAL_SOURCE_DIR}/src/bindings/fluid_cmd.c
        ${EXTERNAL_SOURCE_DIR}/src/bindings/fluid_filerenderer.c
        ${EXTERNAL_SOURCE_DIR}/src/drivers/fluid_adriver.c
        ${EXTERNAL_SOURCE_DIR}/src/drivers/fluid_mdriver.c
        ${EXTERNAL_SOURCE_DIR}/src/gentables/fluid_cb2amp.cpp
        ${EXTERNAL_SOURCE_DIR}/src/gentables/fluid_ct2hz.cpp
        ${EXTERNAL_SOURCE_DIR}/src/gentables/fluid_pan.cpp
        ${EXTERNAL_SOURCE_DIR}/src/gentables/fluid_concave.cpp
        ${EXTERNAL_SOURCE_DIR}/src/gentables/fluid_convex.cpp
        ${EXTERNAL_SOURCE_DIR}/src/gentables/fluid_interp_coeff.cpp
        ${EXTERNAL_SOURCE_DIR}/src/gentables/fluid_interp_coeff_linear.cpp
        ${EXTERNAL_SOURCE_DIR}/src/gentables/fluid_interp_coeff_sinc7.cpp
        ${EXTERNAL_SOURCE_DIR}/src/midi/fluid_midi.c
        ${EXTERNAL_SOURCE_DIR}/src/midi/fluid_midi_router.c
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_adsr_env.c
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_chorus.c
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_iir_filter.c
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_iir_filter_impl.cpp
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_lfo.c
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_rev.c
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_rvoice.c
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_rvoice_dsp.cpp
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_rvoice_event.c
        ${EXTERNAL_SOURCE_DIR}/src/rvoice/fluid_rvoice_mixer.c
        ${EXTERNAL_SOURCE_DIR}/src/sfloader/fluid_defsfont.c
        ${EXTERNAL_SOURCE_DIR}/src/sfloader/fluid_dls.cpp
        ${EXTERNAL_SOURCE_DIR}/src/sfloader/fluid_samplecache.c
        ${EXTERNAL_SOURCE_DIR}/src/sfloader/fluid_sffile.c
        ${EXTERNAL_SOURCE_DIR}/src/sfloader/fluid_sfont.c
        ${EXTERNAL_SOURCE_DIR}/src/synth/fluid_chan.c
        ${EXTERNAL_SOURCE_DIR}/src/synth/fluid_gen.c
        ${EXTERNAL_SOURCE_DIR}/src/synth/fluid_mod.c
        ${EXTERNAL_SOURCE_DIR}/src/synth/fluid_synth.c
        ${EXTERNAL_SOURCE_DIR}/src/synth/fluid_synth_monopoly.c
        ${EXTERNAL_SOURCE_DIR}/src/synth/fluid_tuning.c
        ${EXTERNAL_SOURCE_DIR}/src/synth/fluid_voice.c
        ${EXTERNAL_SOURCE_DIR}/src/utils/fluid_conv.c
        ${EXTERNAL_SOURCE_DIR}/src/utils/fluid_hash.c
        ${EXTERNAL_SOURCE_DIR}/src/utils/fluid_list.c
        ${EXTERNAL_SOURCE_DIR}/src/utils/fluid_ringbuffer.c
        ${EXTERNAL_SOURCE_DIR}/src/utils/fluid_settings.c
        ${EXTERNAL_SOURCE_DIR}/src/utils/fluid_sys.c
        ${EXTERNAL_SOURCE_DIR}/src/utils/fluid_sys_embedded.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/bindings
        ${EXTERNAL_SOURCE_DIR}/src/drivers
        ${EXTERNAL_SOURCE_DIR}/src/midi
        ${EXTERNAL_SOURCE_DIR}/src/rvoice
        ${EXTERNAL_SOURCE_DIR}/src/sfloader
        ${EXTERNAL_SOURCE_DIR}/src/synth
        ${EXTERNAL_SOURCE_DIR}/src/utils

        ${EXTERNAL_SOURCE_DIR_gcem}/include

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE OSAL_embedded=1) #TODO why not using config.h one?
target_compile_definitions(${PROJECT_NAME} PRIVATE FMOD_DLS_PATH="${PLUGINS_FMOD_DIR}/gm.dls") #TODO

target_link_libraries(${PROJECT_NAME} PRIVATE ${MATH_LIBRARY})

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
