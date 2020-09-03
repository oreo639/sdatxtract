#ifndef __FILE_HELPER_H__
#define __FILE_HELPER_H__

typedef struct {
	const uint8_t* data;
	const uint32_t size;

	size_t offset;
} fbuffer_t;

void FILE_mkdir(const char* fpath);
void FILE_chdir(const char* fpath);

fbuffer_t FILE_createFbfFromBuff(uint8_t* in_buff, uint32_t size);
uint32_t FILE_getU32(fbuffer_t* buffer);
uint16_t FILE_getU16(fbuffer_t* buffer);
uint8_t FILE_getU8(fbuffer_t* buffer);
size_t FILE_read(fbuffer_t* buffer, void* in_buff, size_t size);
bool FILE_seek(fbuffer_t* buffer, uint32_t offset);
uint32_t FILE_getOffset(fbuffer_t* buffer);

uint8_t *FILE_loadFileFromFP(FILE* fp_in, uint32_t offset, uint32_t size);
uint8_t *FILE_loadFileFromBuff(uint8_t* in_buff, uint32_t size);
void FILE_outPutFileFromBuff(const char* out, uint8_t* file_buff, uint32_t size);

#endif
