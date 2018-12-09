#include "main.h"


void FILE_mkdir(const char* fpath) {
	#ifdef __MINGW32__
	mkdir(fpath);
	#elif _MSC_VER
	CreateDirectory(fpath, NULL);
	#else
	mkdir(fpath, 0777);
	#endif
}

char *FILE_loadFileFromFP(FILE* fp_in, uint32_t offset, uint32_t size) {
	char* file_buff;
	fseek(fp_in, offset, SEEK_SET);
	file_buff = malloc(size);
	fread(file_buff, 1, size, fp_in);
	return file_buff;
}

void FILE_outPutFileFromBuff(const char* out, char* file_buff, uint32_t size) {
	FILE *fp_out = fopen(out, "wb");
	fwrite(file_buff, 1, size, fp_out);
	fclose(fp_out);
}

uint32_t FILE_getUint(char* data) {
	uint32_t a;
	memcpy(&a, data, sizeof(uint32_t));
	return a;
}

uint16_t FILE_getShort(char* data) {
	uint16_t a;
	memcpy(&a, data, sizeof(uint16_t));
	return a;
}

uint8_t FILE_getU8(char* data) {
	uint8_t a;
	memcpy(&a, data, sizeof(uint8_t));
	return a;
}
