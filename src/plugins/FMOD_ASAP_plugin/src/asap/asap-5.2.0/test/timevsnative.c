#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asap.h"

static int print_times(ASAPInfo *info, const char *filename, const unsigned char *module, int module_len)
{
	int sum = 0;
	int song;
	if (!ASAPInfo_Load(info, filename, module, module_len)) {
		putchar('\n');
		fprintf(stderr, "%s: cannot load\n", filename);
		exit(1);
	}
	for (song = 0; song < ASAPInfo_GetSongs(info); song++) {
		int duration = ASAPInfo_GetDuration(info, song);
		unsigned char s[ASAPWriter_MAX_DURATION_LENGTH + 1];
		int len = ASAPWriter_DurationToString(s, duration);
		sum += duration;
		putchar(song == 0 ? '\t' : ',');
		if (len == 0)
			printf("???");
		else {
			s[len] = '\0';
			fputs((const char *) s, stdout);
			if (ASAPInfo_GetLoop(info, song))
				printf(" LOOP");
		}
	}
	return sum;
}

int main(int argc, char *argv[])
{
	ASAPInfo *sap_info = ASAPInfo_New();
	ASAPInfo *native_info = ASAPInfo_New();
	if (argc != 2) {
		printf("Usage: timevsnative FILE.sap\n");
		return 1;
	}
	const char *sap_filename = argv[1];
	FILE *fp = fopen(sap_filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "%s: cannot open\n", sap_filename);
		return 1;
	}
	unsigned char sap[ASAPInfo_MAX_MODULE_LENGTH];
	int sap_len = fread(sap, 1, sizeof(sap), fp);
	fclose(fp);
	fputs(sap_filename, stdout);
	int sap_sum = print_times(sap_info, sap_filename, sap, sap_len);
	const char *native_ext = ASAPInfo_GetOriginalModuleExt(sap_info, sap, sap_len);
	if (native_ext != NULL) {
		ASAPWriter *writer = ASAPWriter_New();
		printf("\t%s", native_ext);
		char native_filename[FILENAME_MAX];
		sprintf(native_filename, "%.*s.%s", (int) (strrchr(sap_filename, '.') - sap_filename), sap_filename, native_ext);
		unsigned char native[ASAPInfo_MAX_MODULE_LENGTH];
		ASAPWriter_SetOutput(writer, native, 0, sizeof(native));
		int native_len = ASAPWriter_Write(writer, native_filename, sap_info, sap, sap_len, false);
		ASAPWriter_Delete(writer);
		if (native_len >= 0) {
			int native_sum = print_times(native_info, native_filename, native, native_len);
			printf("\t%d", abs(native_sum - sap_sum) / 1000);
		}
	}
	putchar('\n');
	return 0;
}
