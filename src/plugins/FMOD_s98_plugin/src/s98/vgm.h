static BYTE *vgm2s98(BYTE *ps, DWORD slen, DWORD *pdlen)
{
	DWORD spos = 0x40, dpos = 0x80, psloop, clk;
	BYTE *pd;

	if (slen < spos) return 0;

	/* 300%(worst case) */
	pd = (BYTE *)malloc(dpos + (slen + slen + slen) + 1);
	if (!pd) return 0;
	XMEMSET(pd, 0, dpos);

	SetDwordBE(pd + S98_OFS_MAGIC, S98_MAGIC_V2);
	SetDwordLE(pd + S98_OFS_TIMER_INFO1, 1);
	SetDwordLE(pd + S98_OFS_TIMER_INFO2, 44100);
	SetDwordLE(pd + S98_OFS_OFSDATA, dpos);
	clk = GetDwordLE(ps + 0x0c);
	SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x00, S98DEVICETYPE_SNG);
	SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x04, clk ? clk : (53693175/15));
	clk = GetDwordLE(ps + 0x10);
	SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x10, S98DEVICETYPE_OPLL);
	SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x14, clk ? clk : (53693175/15));
	SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x20, S98DEVICETYPE_OPN2);
	SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x24, clk ? clk : (53693175/7));
	SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x30, S98DEVICETYPE_OPM);
	SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x34, clk ? clk : 8000000);

	psloop = GetDwordLE(ps + 0x1c);
	if (psloop) psloop += 0x1c;
	while (spos < slen && ps[spos] != 0x66)
	{
		if (psloop && psloop == spos) SetDwordLE(pd + S98_OFS_OFSLOOP, dpos);
		switch (ps[spos])
		{
			case 0x4f:	/* GG stereo */
			case 0x50:	/* SNG */
				pd[dpos++] = 0;
				pd[dpos++] = 0x50 - ps[spos++];
				pd[dpos++] = ps[spos++];
				break;
			case 0x51:	/* OPLL */
				spos++;
				pd[dpos++] = 2;
				pd[dpos++] = ps[spos++];
				pd[dpos++] = ps[spos++];
				break;
			case 0x52:	/* OPN2 master */
			case 0x53:	/* OPN2 slave */
				pd[dpos++] = 4 + ps[spos++] - 0x52;
				pd[dpos++] = ps[spos++];
				pd[dpos++] = ps[spos++];
				break;
			case 0x54:	/* OPM */
				spos++;
				pd[dpos++] = 6;
				pd[dpos++] = ps[spos++];
				pd[dpos++] = ps[spos++];
				break;
			case 0x55: case 0x56: case 0x57: case 0x58: case 0x59:
			case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				spos += 3;
				break;
			case 0x61:
			case 0x62:
			case 0x63:
				{
					Uint32 wait;
					switch (ps[spos++])
					{
						case 0x61:
							wait = GetWordLE(ps + spos);
							spos += 2;
							break;
						case 0x62:
							wait = 735;
							break;
						case 0x63:
							wait = 882;
							break;
					}
					if (wait == 1)
					{
						pd[dpos++] = 0xff;
					}
					else if (wait == 2)
					{
						pd[dpos++] = 0xff;
						pd[dpos++] = 0xff;
					}
					else if (wait > 2)
					{
						wait -= 2;
						pd[dpos++] = 0xfe;
						while (wait > 0x7f)
						{
							pd[dpos++] = (wait & 0x7f) + 0x80;
							wait >>= 7;
						}
						pd[dpos++] = wait & 0x7f;
					}
				}
				break;
			default:
#if 1
				spos++;
				break;
#else
				free(pd);
				return 0;
#endif
		}
	}
	pd[dpos++] = 0xfd;
	*pdlen = dpos;
	return pd;
}
