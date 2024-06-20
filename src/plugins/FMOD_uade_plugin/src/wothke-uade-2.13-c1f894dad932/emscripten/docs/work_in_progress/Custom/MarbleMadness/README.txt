NOTE: The MarbleMadness songs seem to play sufficiently well now.

It seems that the modules can be built from source - though some patching is needed to 
make the available assembler happy (BRA.L -> BRA, FOO.MSG -> FOO_MSG, etc). The source code 
in amigasrc/players/madness is a somewhat "beautified" version of WantedTeam's 
original implementation intended to identify what might be the sore spot in UADEs current impl.


Interestingly the implementation of SkyFox is very similar to MarbleMadness with large portions of 
the code being identical - or almost identical with only small modifications. The basic player 
structure with the different Tasks and the used signalling seems to be "identical". 

The modelling of the song-data seems to be quite different: Whereas MarbleMadness uses
many statically interlinked blocks of "Snd" data (see SampleData - with the root at the 
Snd000000 section), SkyFox uses smaller "independent" blocks that are connected during initialization 
by the "InitVob" function (see ptAudEGTable). It seems reasonable to persume that the code variations 
are mainly due to the different data structures.


Seems the main difference to SkyFox is that the CMD_WRITE's use smaller buffers and the next 
request is usually already enqueued before the the previous one has been unqueued:

	beginIO; CMD_WRITE
		add CMD_WRITE
		start CMD_WRITE (with len ranging from 4-64, i.e. many very small; implicit ADIOF_PERVOL)

	beginIO; CMD_WRITE
		add CMD_WRITE
		
		remove CMD_WRITE 
		
	beginIO; CMD_FLUSH
		CMD_FLUSH

=> the earlier impl had the issue that it never started the "start CMD_WRITE" for that 2nd CMD_WRITE
and once that had been fixed, the songs now play (note: the default track #0 seems to be identical
in all songs, i.e. it is normal that all the modules sound the same)
		
=> the next issue was that "preemptive" features for Task scheduling actually seem to be needed and
without then the songs eventually got stuck waiting for signals and were not woken up when these arrived.

With the above fixed the songs now seem to play (there is only 1 sub-track left that still seems to get
stuck eventually but given UADE's underlying limitations it isn't worth the trouble to invest more 
time here..)

One open issue still concerns the memory buffers used in the "CMD_WRITE": Is seems that the player 
uses the same buffer for subsequent messages sent to the same channel. It is possible that this 
actually corrupts buffers that have not yet been played.. It would probably be safer to copy the
buffers that are referenced from respective commands (e.g. the moment the command is enqueued).
The songs definitely sound much crappier than the original Atari arcade version, or how I remembered
them. 
