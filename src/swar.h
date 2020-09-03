#ifndef __SX_SWAREX__H__
#define __SX_SWAREX__H__

typedef struct {
	uint8_t *fileimage;
	uint32_t filesize;
} SWARfile_t;

typedef struct {
	uint8_t *swarimage;
	uint32_t swarsize;
	SWARfile_t *file;
	uint32_t filenum;
} SWAR;

typedef struct {
	uint8_t *swavimage;
	uint32_t swavsize;
} SWAV;

enum swar_error {
	SWARE_OK = 0,
	SWARE_BAD,
	SWARE_EMPTY,
	SWARE_GENERIC
};


int SWAREX_init(SWAR* swarfile, uint8_t *image, uint32_t size);

int SWAV_genSwav(SWARfile_t* swardata, SWAV* swavstruct);

void SWAV_clear(SWAV* swavstruct);

void SWAREX_exit(SWAR* swarstruct);

#endif	//__SX_SWAREX__H__
