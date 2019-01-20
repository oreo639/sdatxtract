#include "main.h"
#include "nds.h"
/*\
|*| Nds documentation can be found here:
|*| http://www.dsibrew.org/wiki/DSi_Cartridge_Header
\*/

bool NDS_isNds(const char* filepath) {
	FILE *fp = fopen(filepath, "rb");
	uint8_t identifier;
	long int size;

	if(!fp){
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if(size < 0x400){
		fclose(fp);
		return false;
	}

	fseek(fp, 0x12, SEEK_SET);
	fread(&identifier, 1, 1, fp);

	fclose(fp);
	verbose("NDS IDENT:%d\n", identifier);
	if (identifier == 0x00 || identifier == 0x02 || identifier == 0x03) {
		return true;
	}
	return false;
}


bool NDS_getGameTitle(const char* filepath, char *buf){
	FILE *fp = fopen(filepath, "rb");
	char invalid_chars[] = {'/','\\','<','>','*','?','\"','|',':',';',' '};
	long int size;

	if(!fp){
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if(size < 0x400){
		fclose(fp);
		return false;
	}
	memset(buf, 0, 13);
	fread(buf, 1, 4, fp);
	buf[12] = '\0';

	//Check for invalid characters.
	for(int i = 0; i < (int)strlen(buf); i ++){
		for(int j = 0; j < sizeof(buf)/sizeof(char *); j ++){
			if(buf[i] == invalid_chars[j]){
				buf[i] = '_';
				break;
			}
		}
	}
	fclose(fp);
	return true;
}

bool NDS_getGameCode(const char* filepath, char *buf){
	FILE *fp = fopen(filepath, "rb");

	if(!fp){
		return false;
	}

	fseek(fp, 0x0C, SEEK_SET);
	fread(buf, 1, 4, fp);
	buf[4] = '\0';

	fclose(fp);
	return true;
}

bool NDS_getSDAToffset(const char* filepath, NDS *ndsdata) {
	FILE *fp = fopen(filepath, "rb");
	long unsigned int size;
	uint8_t *nds_data_tmp;
	uint8_t sdatMagic[] = {'S','D','A','T',0xFF,0xFE,0x00,0x01};

	if(!fp){
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size = (long unsigned int)ftell(fp);
	if(size < 0x400){
		fclose(fp);
		return false;
	}
	nds_data_tmp = malloc(size);
	fseek(fp, 0, SEEK_SET);
	ndsdata->sdatnum = 0;

	ndsdata->ndsfile = malloc(sizeof(NDSfile_t));

	fread(nds_data_tmp, 1, size, fp);
	for (uint32_t i = 0; i + 4 < size; i++) {
		if(!memcmp((nds_data_tmp + i), sdatMagic, sizeof(sdatMagic))) {
			verbose("SDAT found!\n");
			if ((FILE_getShort((nds_data_tmp + i) + 0xC) < 0x100) && (FILE_getUint((nds_data_tmp + i) + 0x10) < 0x10000)) {
				verbose("SDAT confirmed!\n");
				verbose("Reading sdat %d.\n", ndsdata->sdatnum);
				ndsdata->ndsfile = realloc(ndsdata->ndsfile, (ndsdata->sdatnum+0x01) * sizeof(NDSfile_t));
				ndsdata->ndsfile[ndsdata->sdatnum].sdatImage = FILE_loadFileFromBuff(nds_data_tmp + i,
					FILE_getUint(nds_data_tmp + i + 0x08));
				ndsdata->ndsfile[ndsdata->sdatnum].sdatsize = FILE_getUint(nds_data_tmp + i + 0x08);
				verbose("Sdat offset:%d\nSdat Size:%d\n", i, ndsdata->ndsfile[ndsdata->sdatnum].sdatsize);
				ndsdata->sdatnum++;
			}
			else {
				verbose("SDAT rejected!\n");
			}
		}
	}
	free(nds_data_tmp);
	fclose(fp);

	if(ndsdata->sdatnum <= 0) {
		return false;
	}
	printf("Total SDATs found: %d\n", ndsdata->sdatnum);
	return true;
}

bool NDS_dumpSDAT(const char *filepath, const char* fileout, NDSfile_t *ndsfile) {
	FILE *fpout = fopen(fileout, "wb");
	fwrite(ndsfile->sdatImage, 1, ndsfile->sdatsize, fpout);
	fclose(fpout);
	return true;
}
