PlayAY uses DTG_Process to run a separate message-listening loop - which isn't actually 
used except for the ZXAYEMUL case. Except for ZXAYEMUL I got all the other cases to work.


The ZXAYEMUL player then also runs a Z80 emulator in a separate process.. (it currently 
runs as an endless loop from a "blackbox" ).

Currently the Z80-emu process would send DeliMessage structs to the lister to make it
update audio-registers asynchronously:

   STRUCTURE DeliMessage,MN_SIZE
	ULONG	DTMN_Function			; function pointer to the code that should be run
	ULONG	DTMN_Result			; store the result here
   LABEL DTMN_SIZE

that interaction could maybe be patched up to run synchronously.

As long as UADE's kernel does not support multitasking/multiprocessing, the only
workaround would be to patch the player to do everything synchronously: The main 
thing would be to break up the "Z80 emulator loop" in order to somehow
run it synchonously from the UADE playloop.


Since NEZplug already covers the AZAYEMUL format there is really no need to 
get this player to run... moving it to the "abandoned players" pile.