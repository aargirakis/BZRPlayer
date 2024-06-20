kernel void asap2wav(constant char *filename, global const uchar *module, int module_len, int song, int duration, global uchar *wav, int wav_len)
{
	ASAP asap;
	ASAP_Construct(&asap);
	if (!ASAP_Load(&asap, filename, module, module_len)
	 || !ASAP_PlaySong(&asap, song, duration)) {
		wav[0] = '\0';
		return;
	}
	int header_len = ASAP_GetWavHeader(&asap, wav, ASAPSampleFormat_S16_L_E, false);
	ASAP_Generate(&asap, wav + header_len, wav_len - header_len, ASAPSampleFormat_S16_L_E);
}
