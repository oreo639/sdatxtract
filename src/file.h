#ifndef __FILE_HELPER_H__
#define __FILE_HELPER_H__


void FILE_mkdir(const char* fpath);
uint32_t FILE_getUint(uint8_t* data);
uint16_t FILE_getShort(uint8_t* data);
uint8_t FILE_getU8(uint8_t* data);
uint8_t *FILE_loadFileFromFP(FILE* fp_in, uint32_t offset, uint32_t size);
uint8_t *FILE_loadFileFromBuff(uint8_t* in_buff, uint32_t size);
void FILE_outPutFileFromBuff(const char* out, uint8_t* file_buff, uint32_t size);
#endif
