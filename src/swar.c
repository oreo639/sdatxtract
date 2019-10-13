#include "main.h"
#include "swar.h"

int SWAREX_init(SWAR* swar, uint8_t *image, uint32_t size) {
	if(!memcmp(image, "SWAR", 4)){
		swar->swarsize = size;
		swar->swarimage = image;

		swar->filenum = FILE_getUint(swar->swarimage + 0x38);
		swar->file = malloc(sizeof(SWARfile_t) * swar->filenum);

		uint8_t* current_pos = swar->swarimage + 0x3C;
		if (swar->filenum)
		{
			uint32_t i;
			for(i = 0; i < swar->filenum - 1; i ++){
				swar->file[i].fileimage = swar->swarimage + FILE_getUint(current_pos);
				swar->file[i].filesize = FILE_getUint(current_pos + 0x04) - FILE_getUint(current_pos);
				current_pos += 0x04;
			}
			swar->file[i].fileimage = swar->swarimage + FILE_getUint(current_pos);
			swar->file[i].filesize = swar->swarsize - FILE_getUint(current_pos);
			return 0;
		}
		return 2;
	}
	return 1;
}

void SWAREX_exit(SWAR* swar) {
	if (swar->file) free(swar->file);
}

int SWAV_genSwav(SWARfile_t* file, SWAV* swav) {
	swav->swavsize = file->filesize + 0x18;
	swav->swavimage = malloc(swav->swavsize);

	memcpy(swav->swavimage, "SWAV", 4);
	*(uint32_t*)(swav->swavimage + 0x04) = 0x0100FEFF;
	*(uint32_t*)(swav->swavimage + 0x08) = swav->swavsize;
	*(uint32_t*)(swav->swavimage + 0x0C) = 0x00010010;
	memcpy(swav->swavimage + 0x10, "DATA", 4);
	*(uint32_t*)(swav->swavimage + 0x14) = swav->swavsize + 0x08;
	memcpy(swav->swavimage + 0x18, file->fileimage, file->filesize);

	return 0;
}

void SWAV_clear(SWAV* swav) {
	if (swav->swavimage) free(swav->swavimage);
}
