Song requires Task/Signal handling support. Also the song depends on audio.device.

The song also uses exec.library/DoIO to invoke ADCMD_FINISH. => adding
a respective impl seems to have fixed the issue - though the songs 
still sound very crappy (same as the youtube recordings).

Let's just hope there are other songs that also benefit from the addition.