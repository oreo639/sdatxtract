#include "main.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif


void FILE_mkdir(const char* fpath) {
#ifdef _WIN32
	_mkdir(fpath);
#else
	mkdir(fpath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
	verbose("Created directory %s\n", fpath);
}

void FILE_chdir(const char* fpath) {
#ifdef _WIN32
	_chdir(fpath);
#else
	chdir(fpath);
#endif
	verbose("Changed to directory %s\n", fpath);
}

bool FILE_TestOpen(const char* fpath) {
	FILE *fp = fopen(fpath, "rb");
	if (!fp)
		return false;

	fclose(fp);
	return true;
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

fbuffer_t FILE_createFbfFromBuff(uint8_t* in_buff, uint32_t size) {
	fbuffer_t tmp = {in_buff, size, 0};
	return tmp;
}

uint32_t FILE_getU32(fbuffer_t* buffer) {
	if (buffer->offset+sizeof(uint32_t) > buffer->size)
		return 0;

	uint32_t a;
	memcpy(&a, buffer->data + buffer->offset, sizeof(uint32_t));
	buffer->offset+=sizeof(uint32_t);
	return a;
}

uint16_t FILE_getU16(fbuffer_t* buffer) {
	if (buffer->offset+sizeof(uint16_t) > buffer->size)
		return 0;

	uint16_t a;
	memcpy(&a, buffer->data + buffer->offset, sizeof(uint16_t));
	buffer->offset+=sizeof(uint16_t);
	return a;
}

uint8_t FILE_getU8(fbuffer_t* buffer) {
	if (buffer->offset+sizeof(uint8_t) > buffer->size)
		return 0;

	uint8_t a;
	memcpy(&a, buffer->data + buffer->offset, sizeof(uint8_t));
	buffer->offset+=sizeof(uint8_t);
	return a;
}

size_t FILE_read(fbuffer_t* buffer, void* in_buff, size_t size) {
	if (buffer->offset+size > buffer->size)
		return 0;

	memcpy(in_buff, (uint8_t*)buffer->data + buffer->offset, size);
	buffer->offset+=size;
	return size;
}

bool FILE_seek(fbuffer_t* buffer, uint32_t offset) {
	if (offset > buffer->size)
		return false;

	buffer->offset=offset;
	return true;
}

uint32_t FILE_getOffset(fbuffer_t* buffer) {
	return (uint32_t)buffer->offset;
}
