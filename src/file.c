#include "main.h"

#ifdef _MSC_VER
#include <windows.h>
#endif


void FILE_mkdir(const char* fpath) {
	struct stat sb;
	if (stat(fpath, &sb)) {
	#ifdef __MINGW32__
		mkdir(fpath);
	#elif _MSC_VER
		CreateDirectory(fpath, NULL);
	#else
		mkdir(fpath, 0777);
	#endif
	verbose("Created directory %s\n", fpath);
	}
}

uint8_t *FILE_loadFileFromFP(FILE* fp_in, uint32_t offset, uint32_t size) {
	uint8_t* file_buff;
	fseek(fp_in, offset, SEEK_SET);
	file_buff = malloc(size);
	fread(file_buff, 1, size, fp_in);
	return file_buff;
}

uint8_t *FILE_loadFileFromBuff(uint8_t* in_buff, uint32_t size) {
	uint8_t* out_buff;
	out_buff = malloc(size);
	memcpy(out_buff, in_buff, size);
	return out_buff;
}

void FILE_outPutFileFromBuff(const char* out, uint8_t* file_buff, uint32_t size) {
	FILE *fp_out = fopen(out, "wb");
	fwrite(file_buff, 1, size, fp_out);
	fclose(fp_out);
}

uint32_t FILE_getUint(uint8_t* data) {
	uint32_t a;
	memcpy(&a, data, sizeof(uint32_t));
	return a;
}

uint16_t FILE_getShort(uint8_t* data) {
	uint16_t a;
	memcpy(&a, data, sizeof(uint16_t));
	return a;
}

uint8_t FILE_getU8(uint8_t* data) {
	uint8_t a;
	memcpy(&a, data, sizeof(uint8_t));
	return a;
}
