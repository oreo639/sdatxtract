#include "main.h"
#include "sdat.h"
#include "nds.h"

/*\
|*| SDAT documentation can be found here:
|*| http://www.romhacking.net/documents/%5b469%5dnds_formats.htm#SDAT
\*/

bool SDAT_isSDAT(const char *filepath) {
	FILE *fp = fopen(filepath, "rb");
	uint8_t sdatMagic[] = {'S','D','A','T',0xFF,0xFE,0x00,0x01};
	char magic[sizeof(sdatMagic)+1];

	if(!fp){
		return false;
	}

	fseek(fp, 0, SEEK_SET);
	fread(magic, 1, sizeof(sdatMagic), fp);
	fseek(fp, 0, SEEK_SET);

	fclose(fp);

	if(!memcmp(magic, sdatMagic, sizeof(sdatMagic)))
	{
		return true;
	}

	return false;
}

bool SDAT_getUniqueId(const char *filepath, char *buf) {
	FILE *fp = fopen(filepath, "rb");
	uint32_t data;

	if(!fp){
		return false;
	}

	fseek(fp, 0x1C, SEEK_SET);
	fread(&data, 1, 4, fp);
	snprintf(buf, 4, "%d", data);
	buf[4] = '\0';

	fclose(fp);

	return true;
}

bool SDAT_fakeNds(const char *filepath, nds_t *nds) {
	FILE *fp = fopen(filepath, "rb");
	long unsigned int size;
	uint8_t *sdat_data_tmp;

	if(!fp){
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size = (long unsigned int)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	sdat_data_tmp = malloc(size);
	fread(sdat_data_tmp, 1, size, fp);

	if (getUint(sdat_data_tmp + 0x08) != size) {
		printf("Size mismatch detected (file may be corrupted). Continuing.\n");
	}

	nds->sdatfile = malloc(sizeof(sdatfile_t));

	nds->sdatfile[0].sdatImage = FILE_loadFileFromBuff(sdat_data_tmp + 0, getUint(sdat_data_tmp + 0x08));
	nds->sdatfile[0].sdatsize = getUint(sdat_data_tmp + 0x08);
	verbose("Size:%d\n", nds->sdatfile[0].sdatsize);
	nds->sdatnum = 1;
	free(sdat_data_tmp);
	fclose(fp);
	return true;
}

/*void sdatnamefree(sdat_t *nsdat) {
	for (uint32_t i = 0; i < nsdat->sseqName.numFiles; i++)
		free(nsdat->sseqName.name[i]);

	for (uint32_t i = 0; i < nsdat->sbnkName.numFiles; i++)
		free(nsdat->sbnkName.name[i]);

	for (uint32_t i = 0; i < nsdat->swarName.numFiles; i++)
		free(nsdat->swarName.name[i]);

	for (uint32_t i = 0; i < nsdat->strmName.numFiles; i++)
		free(nsdat->strmName.name[i]);

	free(nsdat->strmName.name);
	free(nsdat->swarName.name);
	free(nsdat->sbnkName.name);
	free(nsdat->sseqName.name);
	verbose("Freed text\n");

	free(nsdat->sseqName.ident);
	free(nsdat->sbnkName.ident);
	free(nsdat->swarName.ident);
	free(nsdat->strmName.ident);
	verbose("Freed ident\n");
}

void SDAT_close(sdat_t *nsdat)
{
	sdatnamefree(nsdat);

	free(nsdat->sseqfile);
	free(nsdat->swarfile);
	free(nsdat->sbnkfile);
	free(nsdat->strmfile);
	
	verbose("Freed sdat fat filedata\n");
}*/

void SDATi_readHeader(fbuffer_t* fbuf, sheader_t* soundheader) {
	if (!soundheader)
		return;

	FILE_read(fbuf, soundheader->magic, 4);
	soundheader->magic[4] = 0;
	soundheader->endianness = FILE_getU16(fbuf);
	soundheader->version = FILE_getU16(fbuf);
	soundheader->filesize = FILE_getU32(fbuf);
	soundheader->headersize = FILE_getU16(fbuf);
	soundheader->numblocks = FILE_getU16(fbuf);
}

void SDATi_readSYMBIndex(fbuffer_t* fbuf, symbindex_t* symbi, uint32_t baseOffset) {
	symbi->numEntries = FILE_getU32(fbuf);

	symbi->entryOffsets = malloc(symbi->numEntries*sizeof(uint32_t));
	FILE_read(fbuf, symbi->entryOffsets, symbi->numEntries*sizeof(uint32_t));

	char temp[32];      //that 32 is totally arbitrary, i should change it
	symbi->names = malloc(symbi->numEntries*sizeof(char*));
	for (int i = 0; i < symbi->numEntries; i++)
	{
		if (symbi->entryOffsets[i] != 0)
		{
			memcpy(temp, (fbuf->data + baseOffset) + symbi->entryOffsets[i], sizeof(temp));
			symbi->names[i] = strdup(temp);

			//printf("Read SYMB: %s\n", symbi->entries[i]);
		} else {
			symbi->names[i] = NULL;
		}
	}
}

void SDATi_readSYMB(fbuffer_t* fbuf, symb_t* symb) {
	uint32_t baseOffset = FILE_getOffset(fbuf);

	FILE_read(fbuf, symb->magic, 4);
	symb->magic[4] = 0;
	symb->blocksize = FILE_getU32(fbuf);
	FILE_read(fbuf, symb->indexoffsets, 8*sizeof(uint32_t));
	FILE_read(fbuf, symb->padding, 24);

	FILE_seek(fbuf, baseOffset+symb->indexoffsets[SDATI_SSEQ]);
	SDATi_readSYMBIndex(fbuf, &symb->sseq, baseOffset);

	FILE_seek(fbuf, baseOffset+symb->indexoffsets[SDATI_SBNK]);
	SDATi_readSYMBIndex(fbuf, &symb->sbnk, baseOffset);

	FILE_seek(fbuf, baseOffset+symb->indexoffsets[SDATI_SWAR]);
	SDATi_readSYMBIndex(fbuf, &symb->swar, baseOffset);

	FILE_seek(fbuf, baseOffset+symb->indexoffsets[SDATI_STRM]);
	SDATi_readSYMBIndex(fbuf, &symb->strm, baseOffset);

	FILE_seek(fbuf, baseOffset+symb->blocksize);
}

void SDATi_readINFOIndex(fbuffer_t* fbuf, infoindex_t* infoi) {
	infoi->numEntries = FILE_getU32(fbuf);

	infoi->entryOffsets = malloc(infoi->numEntries*sizeof(uint32_t));
	FILE_read(fbuf, infoi->entryOffsets, infoi->numEntries*sizeof(uint32_t));
}

void SDATi_readSseqINFO(fbuffer_t* fbuf, sseqinfoindex_t* sseqi, uint32_t baseOffset) {
	SDATi_readINFOIndex(fbuf, &sseqi->index);

	sseqi->entries = malloc(sseqi->index.numEntries*sizeof(sseqinfo_t));
	for (int i = 0; i < sseqi->index.numEntries; i++)
	{
		if (sseqi->index.entryOffsets[i] != 0)
		{
			memcpy(&sseqi->entries[i], (fbuf->data + baseOffset) + sseqi->index.entryOffsets[i], sizeof(sseqinfo_t));
		} else {
			sseqi->entries[i].fileId = (uint16_t) -1;
		}
	}
}

void SDATi_readSbnkINFO(fbuffer_t* fbuf, sbnkinfoindex_t* sbnki, uint32_t baseOffset) {
	SDATi_readINFOIndex(fbuf, &sbnki->index);

	sbnki->entries = malloc(sbnki->index.numEntries*sizeof(sbnkinfo_t));
	for (int i = 0; i < sbnki->index.numEntries; i++)
	{
		if (sbnki->index.entryOffsets[i] != 0)
		{
			memcpy(&sbnki->entries[i], (fbuf->data + baseOffset) + sbnki->index.entryOffsets[i], sizeof(sbnkinfo_t));
		}  else {
			sbnki->entries[i].fileId = (uint16_t) -1;
		}
	}
}

void SDATi_readSwarINFO(fbuffer_t* fbuf, swarinfoindex_t* swari, uint32_t baseOffset) {
	SDATi_readINFOIndex(fbuf, &swari->index);

	swari->entries = malloc(swari->index.numEntries*sizeof(swarinfo_t));
	for (int i = 0; i < swari->index.numEntries; i++)
	{
		if (swari->index.entryOffsets[i] != 0)
		{
			memcpy(&swari->entries[i], (fbuf->data + baseOffset) + swari->index.entryOffsets[i], sizeof(sseqinfo_t));
		}  else {
			swari->entries[i].fileId = (uint16_t) -1;
		}
	}
}

void SDATi_readStrmINFO(fbuffer_t* fbuf, strminfoindex_t* strmi, uint32_t baseOffset) {
	SDATi_readINFOIndex(fbuf, &strmi->index);

	strmi->entries = malloc(strmi->index.numEntries*sizeof(strminfo_t));
	for (int i = 0; i < strmi->index.numEntries; i++)
	{
		if (strmi->index.entryOffsets[i] != 0)
		{
			memcpy(&strmi->entries[i], (fbuf->data + baseOffset) + strmi->index.entryOffsets[i], sizeof(strminfo_t));
		}  else {
			strmi->entries[i].fileId = (uint16_t) -1;
		}
	}
}

void SDATi_readINFO(fbuffer_t* fbuf, info_t* sinfo) {
	uint32_t baseOffset = FILE_getOffset(fbuf);

	FILE_read(fbuf, sinfo->magic, 4);
	sinfo->magic[4] = 0;
	sinfo->blocksize = FILE_getU32(fbuf);
	FILE_read(fbuf, sinfo->infoOffsets, 8*sizeof(uint32_t));
	FILE_read(fbuf, sinfo->padding, 24);

	FILE_seek(fbuf, baseOffset+sinfo->infoOffsets[SDATI_SSEQ]);
	SDATi_readSseqINFO(fbuf, &sinfo->sseq, baseOffset);

	FILE_seek(fbuf, baseOffset+sinfo->infoOffsets[SDATI_SBNK]);
	SDATi_readSbnkINFO(fbuf, &sinfo->sbnk, baseOffset);

	FILE_seek(fbuf, baseOffset+sinfo->infoOffsets[SDATI_SWAR]);
	SDATi_readSwarINFO(fbuf, &sinfo->swar, baseOffset);

	FILE_seek(fbuf, baseOffset+sinfo->infoOffsets[SDATI_STRM]);
	SDATi_readStrmINFO(fbuf, &sinfo->strm, baseOffset);

	FILE_seek(fbuf, baseOffset+sinfo->blocksize);
}

void SDATi_readFAT(fbuffer_t* fbuf, fat_t* sfat) {
	uint32_t baseOffset = FILE_getOffset(fbuf);

	FILE_read(fbuf, sfat->magic, 4);
	sfat->magic[4] = 0;
	sfat->blocksize = FILE_getU32(fbuf);
	sfat->numEntries = FILE_getU32(fbuf);

	DEBUG("%d\n", sfat->numEntries);

	sfat->entries = malloc(sfat->numEntries*sizeof(fatentry_t));
	for (int i = 0; i < sfat->numEntries; i++) {
		sfat->entries[i].offset = FILE_getU32(fbuf);
		sfat->entries[i].size   = FILE_getU32(fbuf);
		FILE_read(fbuf, sfat->entries[i].padding, 8);

		sfat->entries[i].file = (fbuf->data + sfat->entries[i].offset);
	}

	FILE_seek(fbuf, baseOffset+sfat->blocksize);
}

bool SDAT_getFiles(const char *filepath, sdatfile_t *sdatfile, sdat_t* nsdat) {
	uint32_t u32_tmp, sseqSYMBlist = 0, sbnkSYMBlist = 0, swarSYMBlist = 0, strmSYMBlist = 0;
	uint32_t INFOoffs, INFOlen, SYMBoffs, SYMBlen, FAToffs, FATlen;
	sheader_t soundheader;

	//printf("OFS:%d\nSIZE:%d\n", sdatfile->sdatoffset, sdatfile->sdatsize);

	fbuffer_t fbuf = FILE_createFbfFromBuff(sdatfile->sdatImage, sdatfile->sdatsize);
	fbuffer_t *fp = &fbuf;

	SDATi_readHeader(&fbuf, &soundheader);
	SYMBoffs = FILE_getU32(fp);
	SYMBlen  = FILE_getU32(fp);
	INFOoffs = FILE_getU32(fp);
	INFOlen  = FILE_getU32(fp);
	FAToffs  = FILE_getU32(fp);
	FATlen   = FILE_getU32(fp);
	FILE_getU16(fp); //padding

	if(INFOoffs == 0) {
		printf("Info not found. Cannot continue.\n");
		return false;
	}

	if(FAToffs == 0) {
		printf("FAT not found. Cannot continue.\n");
		return false;
	}

	if(SYMBoffs != 0 || SYMBlen != 0) {
		FILE_seek(&fbuf, SYMBoffs);
		SDATi_readSYMB(&fbuf, &nsdat->symb);
	} else {
		memset(&nsdat->symb, 0, sizeof(symb_t));
		printf("[WARN]: SYMB missing! Filenames will not be set.\n");
	}

	FILE_seek(&fbuf, INFOoffs);
	SDATi_readINFO(&fbuf, &nsdat->info);

	FILE_seek(&fbuf, FAToffs);
	SDATi_readFAT(&fbuf, &nsdat->fat);

	return true;
}
