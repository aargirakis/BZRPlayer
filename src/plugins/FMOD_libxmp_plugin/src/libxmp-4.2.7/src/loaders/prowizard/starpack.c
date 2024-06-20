/*
 * StarTrekker_Packer.c   Copyright (C) 1997 Sylvain "Asle" Chipaux
 *                        Copyright (C) 2006-2009 Claudio Matsuoka
 *
 * Converts back to ptk StarTrekker packed MODs
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_starpack(FILE *in, FILE *out)
{
	uint8 c1, c2, c3, c4, c5;
	uint8 pnum[128];
	uint8 pnum_tmp[128];
	uint8 pat_pos;
	uint8 buffer[1024];
	uint8 num_pat = 0x00;
	int i = 0, j = 0, k = 0;
	int size, ssize = 0;
	int paddr[128];
	int paddr_tmp[128];
	int paddr_tmp2[128];
	int tmp_ptr, tmp1, tmp2;
	int smp_addr = 0;

	memset(pnum, 0, 128);
	memset(pnum_tmp, 0, 128);
	memset(paddr, 0, 128 * 4);
	memset(paddr_tmp, 0, 128 * 4);
	memset(paddr_tmp2, 0, 128 * 4);

	pw_move_data(out, in, 20);		/* title */

	for (i = 0; i < 31; i++) {
		pw_write_zero(out, 22);		/* sample name */
		write16b(out, size = read16b(in));	/* size */
		ssize += 2 * size;
		write8(out, read8(in));		/* finetune */
		write8(out, read8(in));		/* volume */
		write16b(out, read16b(in));	/* loop start */
		write16b(out, read16b(in));	/* loop size */
	}

	pat_pos = read16b(in);			/* size of pattern table */

	fseek(in, 2, SEEK_CUR);			/* bypass $0000 unknown bytes */

	for (i = 0; i < 128; i++)
		paddr[i] = read32b(in);

	/* ordering of patterns addresses */

	tmp_ptr = 0;
	for (i = 0; i < pat_pos; i++) {
		if (i == 0) {
			pnum[0] = 0x00;
			tmp_ptr++;
			continue;
		}

		for (j = 0; j < i; j++) {
			if (paddr[i] == paddr[j]) {
				pnum[i] = pnum[j];
				break;
			}
		}
		if (j == i)
			pnum[i] = tmp_ptr++;
	}

	for (i = 0; i < 128; i++)
		paddr_tmp[i] = paddr[i];

      restart:
	for (i = 0; i < pat_pos; i++) {
		for (j = 0; j < i; j++) {
			if (paddr_tmp[i] < paddr_tmp[j]) {
				tmp2 = pnum[j];
				pnum[j] = pnum[i];
				pnum[i] = tmp2;
				tmp1 = paddr_tmp[j];
				paddr_tmp[j] = paddr_tmp[i];
				paddr_tmp[i] = tmp1;
				goto restart;
			}
		}
	}

	j = 0;
	for (i = 0; i < 128; i++) {
		if (i == 0) {
			paddr_tmp2[j] = paddr_tmp[i];
			continue;
		}

		if (paddr_tmp[i] == paddr_tmp2[j])
			continue;
		paddr_tmp2[++j] = paddr_tmp[i];
	}

	/* try to locate unused patterns .. hard ! */
	j = 0;
	for (i = 0; i < (pat_pos - 1); i++) {
		paddr_tmp[j] = paddr_tmp2[i];
		j += 1;
		if ((paddr_tmp2[i + 1] - paddr_tmp2[i]) > 1024) {
			/*printf ( "! pattern %ld is not used ... saved anyway\n" , j ); */
			paddr_tmp[j] = paddr_tmp2[i] + 1024;
			j += 1;
		}
	}

	/* assign pattern list */
	for (i = 0; i < 128; i++) {
		for (j = 0; j < 128; j++)
			if (paddr[i] == paddr_tmp[j]) {
				pnum_tmp[i] = j;
				break;
			}
	}

	memset(pnum, 0, 128);
	for (i = 0; i < pat_pos; i++) {
		pnum[i] = pnum_tmp[i];
	}

	write8(out, pat_pos);			/* write number of position */

	/* get highest pattern number */
	for (i = 0; i < pat_pos; i++) {
		if (pnum[i] > num_pat)
			num_pat = pnum[i];
	}

	write8(out, 0x7f);			/* write noisetracker byte */
	fwrite(pnum, 128, 1, out);		/* write pattern list */
	write32b(out, PW_MOD_MAGIC);		/* M.K. */

	/* read sample data address */
	fseek(in, 0x310, SEEK_SET);
	smp_addr = read32b(in) + 0x314;

	/* pattern data */
	num_pat += 1;
	for (i = 0; i < num_pat; i++) {
		memset(buffer, 0, 1024);
		for (j = 0; j < 64; j++) {
			for (k = 0; k < 4; k++) {
				int ofs = j * 16 + k * 4;
				c1 = read8(in);
				if (c1 == 0x80)
					continue;
				c2 = read8(in);
				c3 = read8(in);
				c4 = read8(in);
				buffer[ofs] = c1 & 0x0f;
				buffer[ofs + 1] = c2;
				buffer[ofs + 2] = c3 & 0x0f;
				buffer[ofs + 3] = c4;

				c5 = ((c1 & 0xf0) | ((c3 >> 4) & 0x0f)) >> 2;
				buffer[ofs] |= c5 & 0xf0;
				buffer[ofs + 2] |= (c5 << 4) & 0xf0;
			}
		}
		fwrite(buffer, 1024, 1, out);
		/*printf ( "+" ); */
	}
	/*printf ( "\n" ); */

	/* sample data */
	fseek(in, smp_addr, 0);
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_starpack(uint8 *data, char *t, int s)
{
	int start = 0;
	int j, k, l, m;
	int ssize;

#if 0
	/* test 1 */
	if (i < 23) {
		Test = BAD;
		return;
	}
#endif

	/* test 2 */
	l = (data[start + 268] << 8) + data[start + 269];
	if (l & 0x03)
		return -1;

	k = l / 4;
	if (k == 0 || k > 127)
		return -1;

	if (data[start + 784] != 0)
		return -1;

	/* test #3  smp size < loop start + loop size ? */
	/* l is still the size of the pattern list */
	for (k = 0; k < 31; k++) {
		j = (((data[start + 20 + k * 8] << 8) + data[start + 21 + k * 8]) * 2);
		ssize = (((data[start + 24 + k * 8] << 8) + data[start + 25 + k * 8]) * 2) + (((data[start + 26 + k * 8] << 8) + data[start + 27 + k * 8]) * 2);

		if ((j + 2) < ssize)
			return -1;
	}

	/* test #4  finetunes & volumes */
	/* l is still the size of the pattern list */
	for (k = 0; k < 31; k++) {
		if ((data[start + 22 + k * 8] > 0x0f) || (data[start + 23 + k * 8] > 0x40))
			return -1;
	}

	/* test #5  pattern addresses > sample address ? */
	/* l is still the size of the pattern list */
	/* get sample data address */
#if 0
	if ((start + 0x314) > in_size) {
/*printf ( "#5,-1 (Start:%ld)\n" , start );*/
		Test = BAD;
		return;
	}
#endif
	/* k gets address of sample data */
	k = (data[start + 784] << 24)
		+ (data[start + 785] << 16)
		+ (data[start + 786] << 8)
		+ data[start + 787];
#if 0
	if ((k + start) > in_size) {
		Test = BAD;
		return;
	}
#endif
	if (k < 788)
		return -1;

	/* k is the address of the sample data */
	/* pattern addresses > sample address ? */
	for (j = 0; j < l; j += 4) {
		/* m gets each pattern address */ m =
			(data[start + 272 + j] << 24) +
			(data[start + 273 + j] << 16)
			+ (data[start + 274 + j] << 8)
			+ data[start + 275 + j];
		if (m > k)
			return -1;
	}

	/* test last patterns of the pattern list == 0 ? */
	for (j += 2; j < 128; j++) {
		m = (data[start + 272 + j * 4] << 24) +
			(data[start + 273 + j * 4] << 16) +
			(data[start + 274 + j * 4] << 8)
			+ data[start + 275 + j * 4];
		if (m != 0)
			return -1;
	}

	/* test pattern data */
	/* k is the address of the sample data */
	j = start + 788;
	/* j points on pattern data */
	while (j < (k + start - 4)) {
		if (data[j] == 0x80) {
			j += 1;
			continue;
		}

		if (data[j] > 0x80)
			return -1;

		/* empty row ? ... not ptk_tableible ! */
		if ((data[j] == 0x00) &&
			(data[j + 1] == 0x00) &&
			(data[j + 2] == 0x00) &&
			(data[j + 3] == 0x00)) {
			return - 1;
		}

		/* fx = C .. arg > 64 ? */
		if (((data[j + 2] * 0x0f) == 0x0C) && (data[j + 3] > 0x40))
			return - 1;

		/* fx = D .. arg > 64 ? */
		if (((data[j + 2] * 0x0f) == 0x0D) && (data[j + 3] > 0x40))
			return - 1;

		j += 4;
	}

	pw_read_title(data, t, 20);

	return 0;
}

const struct pw_format pw_starpack = {
	"Startrekker Packer",
	test_starpack,
	depack_starpack
};
