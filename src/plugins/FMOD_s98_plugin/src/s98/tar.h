/* TAR file header */
#define TAR_MAGIC	(0x75737461)	/* 'usta' */
#define TAR_MAGIC4	(0x72)			/* 'r' */

static DWORD GetTarOcts(BYTE *p, DWORD l)
{
	DWORD i, r;
	for (i = 0; i < l && p[i] == 0x20; i++);	/* skip space */
	for (r = 0; i < l && '0' <= p[i] && p[i] <= '7'; i++)
	{
		r *= 8;
		r += p[i] - '0';
	}
	return r;
}

static DWORD IsTarHeader(BYTE *p)
{
	DWORD i, sum1, sum2;
	/* magic check */
	if (GetDwordBE(p + 0x101) != TAR_MAGIC || p[0x105] != TAR_MAGIC4) return 0;
	/* sum cehck */
	sum1 = GetTarOcts(p + 0x94, 8);
	sum2 = 0;
	for (i = 0; i < 512; i++) sum2 += (0x94 <= i && i < 0x9C) ? 0x20 : p[i];
	if ((sum1 & 0xFFFF) != (sum2 & 0xFFFF)) return 0;
	if (p[i] != 0x9C) return 512;
	/* check next header */
	sum1 = 512 + ((GetTarOcts(p + 0x7c, 12) + 511) & (~511));
	sum2 = IsTarHeader(p + sum1);
	return sum1 ? (sum1 + sum2) : 0;
}
