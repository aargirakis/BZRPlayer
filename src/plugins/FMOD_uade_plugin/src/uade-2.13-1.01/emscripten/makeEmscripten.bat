emcc.bat  --pre-js pre.js --js-library callback.js -s VERBOSE=0 -Wno-pointer-sign -I../src/ -I../src/include -I../src/frontends/common -Os -O3 --closure 1 --llvm-lto 1 ../src/newcpu.c ../src/memory.c ../src/custom.c ../src/cia.c ../src/audio.c ../src/compiler.c ../src/cpustbl.c ../src/missing.c ../src/sd-sound.c ../src/md-support.c ../src/cfgfile.c ../src/fpp.c ../src/readcpu.c ../src/cpudefs.c ../src/cpuemu1.c ../src/cpuemu2.c ../src/cpuemu3.c ../src/cpuemu4.c ../src/cpuemu5.c ../src/cpuemu6.c ../src/cpuemu7.c ../src/cpuemu8.c ../src/uade.c ../src/unixatomic.c ../src/ossupport.c ../src/uademain.c ../src/sinctable.c ../src/text_scope.c ../src/frontends/common/effects.c ../src/frontends/common/uadeconf.c ../src/frontends/common/support.c ../src/frontends/common/songinfo.c ../src/frontends/common/songdb.c ../src/frontends/common/amifilemagic.c ../src/frontends/common/eagleplayer.c adapter.c -s EXPORTED_FUNCTIONS="['_alloc','_emu_prepare','_emu_init','_emu_is_exit','_emu_compute_audio_samples','_emu_teardown','_emu_get_audio_buffer','_emu_change_subsong','_emu_get_audio_buffer_length']"  -o htdocs/web_uade.html && del htdocs\web_uade.html









