#ifndef __FILE_HELPER_H__
#define __FILE_HELPER_H__


void FILE_mkdir(const char* fpath);
uint32_t FILE_getUint(char* data);
uint16_t FILE_getShort(char* data);
uint8_t FILE_getU8(char* data);
char *FILE_loadFileFromFP(FILE* fp_in, uint32_t offset, uint32_t size);
char *FILE_loadFileFromBuff(char* in_buff, uint32_t size);
void FILE_outPutFileFromBuff(const char* out, char* file_buff, uint32_t size);
#endif
