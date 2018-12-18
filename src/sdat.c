#include "main.h"
#include "sdat.h"
#include "nds.h"
#include "decoder/nsstrm.h"
#include "decoder/sseq2mid.h"

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
	long int size;
	uint32_t data;
	
	if(!fp){
		return false;
	}
	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if(size < 0x400){
		fclose(fp);
		return false;
	}
	fseek(fp, 0x1C, SEEK_SET);
	fread(&data, 1, 4, fp);
	snprintf(buf, 4, "%d", data);
	buf[4] = '\0';
	
	fclose(fp);

	return true;
}

bool SDAT_fakeNds(const char *filepath, NDS *ndsdata) {
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
	
	if (FILE_getUint(sdat_data_tmp + 0x08) != size) {
		printf("Size mismatch detected (file may be corrupted). Continuing.\n");
	}
	
	ndsdata->ndsfile = malloc(sizeof(NDSfile_t));
	
	ndsdata->sdatnum = 0;
	ndsdata->ndsfile[ndsdata->sdatnum].sdatoffset = 0x00; //0 is obviously the begining if it is an SDAT file.
	ndsdata->ndsfile[ndsdata->sdatnum].sdatsize = FILE_getUint(sdat_data_tmp + 0x08);
	verbose("Location:%d\nSize:%d\n", ndsdata->ndsfile[ndsdata->sdatnum].sdatoffset, ndsdata->ndsfile[ndsdata->sdatnum].sdatoffset);
	free(sdat_data_tmp);
	ndsdata->sdatnum++;
	fclose(fp);
	return true;
}

void sdatnamefree(SDAT *sdatfile) {
	/* Clear strings */
	for(uint32_t i = 0; i < sdatfile->sseqName.numFiles; i++)
		free(sdatfile->sseqName.name[i]);

	for(uint32_t i = 0; i < sdatfile->sbnkName.numFiles; i++)
		free(sdatfile->sbnkName.name[i]);
	
	for(uint32_t i = 0; i < sdatfile->swarName.numFiles; i++)
		free(sdatfile->swarName.name[i]);

	for(uint32_t i = 0; i < sdatfile->strmName.numFiles; i++)
		free(sdatfile->strmName.name[i]);

	verbose("Freed text\n");
	free(sdatfile->sseqName.ident);
	free(sdatfile->sbnkName.ident);
	free(sdatfile->swarName.ident);
	free(sdatfile->strmName.ident);
	verbose("Freed ident\n");
}

void SDAT_close(SDAT *sdatfile)
{
	sdatnamefree(sdatfile);
	free(sdatfile->sseqfile);
	free(sdatfile->swarfile);
	free(sdatfile->sbnkfile);
	free(sdatfile->strmfile);
	verbose("Freed sdat fat filedata\n");
}

bool SDAT_getFiles(const char *filepath, NDSfile_t *ndsfile, SDAT* sdatfile) {
	FILE *fp = fopen(filepath, "rb");
	uint8_t *sdat_data_tmp;
	uint32_t u32_tmp, sseqSYMBlist = 0, sbnkSYMBlist = 0, swarSYMBlist = 0, strmSYMBlist = 0, INFOoffs, SYMBoffs, FAToffs;
	
	//printf("OFS:%d\nSIZE:%d\n", ndsfile->sdatoffset, ndsfile->sdatsize);
	fseek(fp, ndsfile->sdatoffset, SEEK_SET);
	sdat_data_tmp = malloc(ndsfile->sdatsize);
	fread(sdat_data_tmp, 1, ndsfile->sdatsize, fp);
	
	SYMBoffs = FILE_getUint(sdat_data_tmp + 0x10);
	
	INFOoffs = FILE_getUint(sdat_data_tmp + 0x18);
	
	FAToffs = FILE_getUint(sdat_data_tmp + 0x20);
	
	if(SYMBoffs == 0) {
		printf("[WARN]: SYMB missing! Filenames will not be set.\n");
	}
	
	if(INFOoffs == 0) {
		printf("Info not found. Cannot continue.\n");
		free(sdat_data_tmp);
		fclose(fp);
		return false;
	}
	
	if(FAToffs == 0) {
		printf("FAT not found. Cannot continue.\n");
		free(sdat_data_tmp);
		fclose(fp);
		return false;
	}
	
	uint32_t sseqsNum = FILE_getUint((sdat_data_tmp + INFOoffs) + FILE_getUint((sdat_data_tmp + INFOoffs) + 0x08));
	
	uint32_t sbnksNum = FILE_getUint((sdat_data_tmp + INFOoffs) + FILE_getUint((sdat_data_tmp + INFOoffs) + 0x10));
	
	uint32_t swarsNum = FILE_getUint((sdat_data_tmp + INFOoffs) + FILE_getUint((sdat_data_tmp + INFOoffs) + 0x14));
	
	uint32_t strmsNum = FILE_getUint((sdat_data_tmp + INFOoffs) + FILE_getUint((sdat_data_tmp + INFOoffs) + 0x24));
	
	verbose("sseqsNum:%d\n", sseqsNum);
	verbose("sbnksNum:%d\n", sbnksNum);
	verbose("swarsNum:%d\n", swarsNum);
	verbose("strmsNum:%d\n", strmsNum);
	
	if (SYMBoffs != 0) {
		/*Sequence symbol List*/
		sseqSYMBlist = FILE_getUint((sdat_data_tmp + SYMBoffs) + 0x08);

		/* Bank symbol list*/
		sbnkSYMBlist = FILE_getUint((sdat_data_tmp + SYMBoffs) + 0x10);

		/*Wav Arcive symbol list*/
		swarSYMBlist = FILE_getUint((sdat_data_tmp + SYMBoffs) + 0x14);
		
		/*Stream symbol list*/
		strmSYMBlist = FILE_getUint((sdat_data_tmp + SYMBoffs) + 0x24);
	}
	
	sdatfile->sseqName.name = malloc((sseqsNum + 1) * sizeof(char*));
	sdatfile->sbnkName.name = malloc((sbnksNum + 1) * sizeof(char*));
	sdatfile->swarName.name = malloc((swarsNum + 1) * sizeof(char*));
	sdatfile->strmName.name = malloc((strmsNum + 1) * sizeof(char*));
	
	for (uint32_t i = 0; i < sseqsNum; i++) {
		char temp[32];        //that 32 is totally arbitrary, i should change it
		if (SYMBoffs != 0 && sseqSYMBlist != 0 && bUseFname == true) {
			u32_tmp = FILE_getUint((sdat_data_tmp + SYMBoffs) + (sseqSYMBlist + 4 + i * 4));
			if (u32_tmp != 0) {
				memcpy(&temp, (sdat_data_tmp + SYMBoffs) + u32_tmp, 32);
			} else {
				snprintf(temp, 32, "SSEQ_%04d", i);
			}
		}
		else {
			snprintf(temp, 32, "SSEQ_%04d", i);
		}
		sdatfile->sseqName.name[i] = strdup(temp);
	}
	
	for (uint32_t i = 0; i < sbnksNum; i++) {
		char temp[32];        //that 32 is totally arbitrary, i should change it
		if (SYMBoffs != 0 && sbnkSYMBlist != 0 && bUseFname == true) {
			u32_tmp = FILE_getUint((sdat_data_tmp + SYMBoffs) + (sbnkSYMBlist + 4 + i * 4));
			if (u32_tmp != 0) {
				memcpy(&temp, (sdat_data_tmp + SYMBoffs) + u32_tmp, 32);
			} else {
				snprintf(temp, 32, "SBNK_%04d", i);
			}
		}
		else {
			snprintf(temp, 32, "SBNK_%04d", i);
		}
		sdatfile->sbnkName.name[i] = strdup(temp);
	}
	
	for (uint32_t i = 0; i < swarsNum; i++) {
		char temp[32];        //that 32 is totally arbitrary, i should change it
		if (SYMBoffs != 0 && swarSYMBlist != 0 && bUseFname == true) {
			u32_tmp = FILE_getUint((sdat_data_tmp + SYMBoffs) + (swarSYMBlist + 4 + i * 4));
			if (u32_tmp != 0) {
				memcpy(&temp, (sdat_data_tmp + SYMBoffs) + u32_tmp, 32);
			} else {
				snprintf(temp, 32, "SWAR_%04d", i);
			}
		}
		else {
			snprintf(temp, 32, "SWAR_%04d", i);
		}
		sdatfile->swarName.name[i] = strdup(temp);
	}
	
	for (uint32_t i = 0; i < strmsNum; i++) {
		char temp[32];        //that 32 is totally arbitrary, i should change it
		if (SYMBoffs != 0 && strmSYMBlist != 0 && bUseFname == true) {
			u32_tmp = FILE_getUint((sdat_data_tmp + SYMBoffs) + (strmSYMBlist + 4 + i * 4));
			if (u32_tmp != 0) {
				memcpy(&temp, (sdat_data_tmp + SYMBoffs) + u32_tmp, 32);
			} else {
				snprintf(temp, 32, "STRM_%04d", i);
			}
		}
		else {
			snprintf(temp, 32, "STRM_%04d", i);
		}
		sdatfile->strmName.name[i] = strdup(temp);
	}
	
	sdatfile->sseqName.numFiles = sseqsNum;
	sdatfile->sbnkName.numFiles = sbnksNum;
	sdatfile->swarName.numFiles = swarsNum;
	sdatfile->strmName.numFiles = strmsNum;
	
	/*Get fileid's*/
	uint32_t pSeqInfoPtrList = FILE_getUint((sdat_data_tmp + INFOoffs) + 0x08);
	//uint32_t seqInfoPtrListLength = FILE_getUint((sdat_data_tmp + INFOoffs) + 0x12);
	uint32_t nSeqInfos = FILE_getUint((sdat_data_tmp + INFOoffs) + pSeqInfoPtrList);
	sdatfile->sseqName.ident = malloc((nSeqInfos + 1) * sizeof(uint16_t*));
	
	for (uint32_t i = 0; i < nSeqInfos; i++) {
		uint32_t pSeqInfo = FILE_getUint((sdat_data_tmp + INFOoffs) + (pSeqInfoPtrList + 4 + i * 4));
		if (pSeqInfo == 0)
			sdatfile->sseqName.ident[i] = (uint16_t) -1;
		else
			sdatfile->sseqName.ident[i] = FILE_getShort((sdat_data_tmp + INFOoffs) + pSeqInfo);
		sdatfile->sseqName.banks = FILE_getShort((sdat_data_tmp + INFOoffs) + pSeqInfo);
		//next bytes would be vol, cpr, ppr, and ply respectively, whatever the heck those last 3 stand for
	}

	uint32_t pBnkInfoPtrList = FILE_getUint((sdat_data_tmp + INFOoffs) + 0x10);
	uint32_t nBnkInfos = FILE_getUint((sdat_data_tmp + INFOoffs) + pBnkInfoPtrList);
	sdatfile->sbnkName.ident = malloc((nBnkInfos + 1) * sizeof(uint16_t*));
	
	for (uint32_t i = 0; i < nBnkInfos; i++) {
		uint32_t pBnkInfo = FILE_getUint((sdat_data_tmp + INFOoffs) + (pBnkInfoPtrList + 4 + i * 4));
		if (pBnkInfo == 0)
			sdatfile->sbnkName.ident[i] = (uint16_t) -1;
		else
			sdatfile->sbnkName.ident[i] = FILE_getShort((sdat_data_tmp + INFOoffs) + pBnkInfo);
	}
	
	uint32_t pWAInfoList = FILE_getUint((sdat_data_tmp + INFOoffs) + 0x14);
	uint32_t nWAInfos = FILE_getUint((sdat_data_tmp + INFOoffs) + pWAInfoList);
	sdatfile->swarName.ident = malloc((nWAInfos + 1) * sizeof(uint16_t*));
	
	for (uint32_t i = 0; i < nWAInfos; i++) {
		uint32_t pWAInfo = FILE_getUint((sdat_data_tmp + INFOoffs) + (pWAInfoList + 4 + i * 4));
		if (pWAInfo == 0)
			sdatfile->swarName.ident[i] = (uint16_t) -1;
		else
			sdatfile->swarName.ident[i] = FILE_getShort((sdat_data_tmp + INFOoffs) + pWAInfo);
	}
	
	uint32_t pStrmInfoList = FILE_getUint((sdat_data_tmp + INFOoffs) + 0x24);
	uint32_t nStrmInfos = FILE_getUint((sdat_data_tmp + INFOoffs) + pStrmInfoList);
	sdatfile->strmName.ident = malloc((nStrmInfos + 1) * sizeof(uint16_t*));
	
	for (uint32_t i = 0; i < nStrmInfos; i++) {
		uint32_t nStrmInfo = FILE_getUint((sdat_data_tmp + INFOoffs) + (pStrmInfoList + 4 + i * 4));
		if (nStrmInfo == 0)
			sdatfile->strmName.ident[i] = (uint16_t) -1;
		else
			sdatfile->strmName.ident[i] = FILE_getShort((sdat_data_tmp + INFOoffs) + nStrmInfo);
	}	

	sdatfile->files = FILE_getUint((sdat_data_tmp + FAToffs) + 0x08);
	sdatfile->sseqfile = malloc(sizeof(SDAT_FILE) * sdatfile->sseqName.numFiles);
	sdatfile->swarfile = malloc(sizeof(SDAT_FILE) * sdatfile->swarName.numFiles);
	sdatfile->sbnkfile = malloc(sizeof(SDAT_FILE) * sdatfile->sbnkName.numFiles);
	sdatfile->strmfile = malloc(sizeof(SDAT_FILE) * sdatfile->strmName.numFiles);
	
	
	for(uint32_t i = 0; i < sdatfile->sseqName.numFiles; i++){
		if (sdatfile->sseqName.ident[i] == (uint16_t) -1) {
			sdatfile->sseqfile[i].filesize = 0;
			continue;
			}
		uint32_t current_pos = FAToffs + (uint32_t)(12 + sdatfile->sseqName.ident[i] * 0x10);
		sdatfile->sseqfile[i].fileoffset = FILE_getUint((sdat_data_tmp + current_pos));
		sdatfile->sseqfile[i].filesize = FILE_getUint((sdat_data_tmp + current_pos) + 0x04);
		sdatfile->sseqfile[i].file = FILE_loadFileFromBuff(sdat_data_tmp + sdatfile->sseqfile[i].fileoffset, sdatfile->sseqfile[i].filesize);
	}

	for(uint32_t i = 0; i < sdatfile->swarName.numFiles; i++){
		if (sdatfile->swarName.ident[i] == (uint16_t) -1) {
			sdatfile->swarfile[i].filesize = 0;
			continue;
			}
		uint32_t current_pos = FAToffs + (uint32_t)(12 + sdatfile->swarName.ident[i] * 0x10);
		sdatfile->swarfile[i].fileoffset = FILE_getUint((sdat_data_tmp + current_pos));
		sdatfile->swarfile[i].filesize = FILE_getUint((sdat_data_tmp + current_pos) + 0x04);
		sdatfile->swarfile[i].file = FILE_loadFileFromBuff(sdat_data_tmp + sdatfile->swarfile[i].fileoffset, sdatfile->swarfile[i].filesize);
	} 

	for(uint32_t i = 0; i < sdatfile->sbnkName.numFiles; i++){
		if (sdatfile->sbnkName.ident[i] == (uint16_t) -1) {
			sdatfile->sbnkfile[i].filesize = 0;
			continue;
			}
		uint32_t current_pos = FAToffs + (uint32_t)(12 + sdatfile->sbnkName.ident[i] * 0x10);
		sdatfile->sbnkfile[i].fileoffset = FILE_getUint((sdat_data_tmp + current_pos));
		sdatfile->sbnkfile[i].filesize = FILE_getUint((sdat_data_tmp + current_pos) + 0x04);
		sdatfile->sbnkfile[i].file = FILE_loadFileFromBuff(sdat_data_tmp + sdatfile->sbnkfile[i].fileoffset, sdatfile->sbnkfile[i].filesize);
	}

	for(uint32_t i = 0; i < sdatfile->strmName.numFiles; i++){
		if (sdatfile->strmName.ident[i] == (uint16_t) -1) {
			sdatfile->strmfile[i].filesize = 0;
			continue;
			}
		uint32_t current_pos = FAToffs + (uint32_t)(12 + sdatfile->strmName.ident[i] * 0x10);
		sdatfile->strmfile[i].fileoffset = FILE_getUint((sdat_data_tmp + current_pos));
		sdatfile->strmfile[i].filesize = FILE_getUint((sdat_data_tmp + current_pos) + 0x04);
		sdatfile->strmfile[i].file = FILE_loadFileFromBuff(sdat_data_tmp + sdatfile->strmfile[i].fileoffset, sdatfile->strmfile[i].filesize);
	}

	free(sdat_data_tmp);
	fclose(fp);
	return true;
}


void SDAT_outputFiles(const char* filepath, const char* outputdir_part1, SDAT* sdatfile) {
	char outputfile[MAX_PATH + 1], outputsseq[MAX_PATH - 34], outputsbnk[MAX_PATH - 34], outputstrm[MAX_PATH - 34], outputswar[MAX_PATH - 34];
	//char *file_buff;
	FILE *fp_in = fopen(filepath, "rb");
	/*SWAR *swar = NULL;
	SWAV *swav = NULL;
	NSSwav *nsswav = NULL;
	*/
	NSStrm *nsstrm = NULL;
	Sseq2mid *nssseq = NULL;
	int numSSEQ = 0, numSTRM = 0, numSWAR = 0, numSBNK = 0;
	
	snprintf(outputsseq, MAX_PATH-35, "%s/%s", outputdir_part1, DIR_SSEQ);
	snprintf(outputsbnk, MAX_PATH-35, "%s/%s", outputdir_part1, DIR_SBNK);
	snprintf(outputstrm, MAX_PATH-35, "%s/%s", outputdir_part1, DIR_STRM);
	snprintf(outputswar, MAX_PATH-35, "%s/%s", outputdir_part1, DIR_SWAR);
	FILE_mkdir(outputsseq);
	FILE_mkdir(outputsbnk);
	FILE_mkdir(outputstrm);
	FILE_mkdir(outputswar);
	
	for (uint32_t i = 0; i < sdatfile->sseqName.numFiles; i++) {
		if (sdatfile->sseqfile[i].filesize == 0)
			continue;
		
		if(bDecodeFile){
			snprintf(outputfile, MAX_PATH, "%s/%s.midi", outputsseq, sdatfile->sseqName.name[i]);
			nssseq = sseq2midCreate(sdatfile->sseqfile[i].file, sdatfile->sseqfile[i].filesize, false);
			if(!nssseq){
				printf("SSEQ open error.\n");
				continue;
			}
			sseq2midSetLoopCount(nssseq, 1);
			sseq2midNoReverb(nssseq, false);
			if(!sseq2midConvert(nssseq)){
				printf("SSEQ convert error.\n");
				sseq2midDelete(nssseq);
				continue;
			}
			if(!sseq2midWriteMidiFile(nssseq, outputfile)){
				printf("MIDI write error.\n");
				sseq2midDelete(nssseq);
				continue;
			}
			else numSSEQ++;
		} else {
			//determine filename
			snprintf(outputfile, MAX_PATH, "%s/%s.sseq", outputsseq, sdatfile->sseqName.name[i]);
			FILE_outPutFileFromBuff(outputfile, sdatfile->sseqfile[i].file, sdatfile->sseqfile[i].filesize);
			numSSEQ++;
		}
		free(sdatfile->sseqfile[i].file);
	}
	
	for (uint32_t i = 0; i < sdatfile->sbnkName.numFiles; i++) {
		if (sdatfile->sbnkfile[i].filesize == 0)
			continue;	
		
		//determine filename
		snprintf(outputfile, MAX_PATH, "%s/%s.sbnk", outputsbnk, sdatfile->sbnkName.name[i]);
		//FO_outPutFileFromFP(fp_in, outputfile, sdatfile->swarfile[i].fileoffset, sdatfile->swarfile[i].filesize);
		
		FILE_outPutFileFromBuff(outputfile, sdatfile->sbnkfile[i].file, sdatfile->sbnkfile[i].filesize);
		free(sdatfile->sbnkfile[i].file);
		numSBNK++;
	}
	
	for (uint32_t i = 0; i < sdatfile->strmName.numFiles; i++) {
		if (sdatfile->strmfile[i].filesize == 0)
			continue;
		
		if(bDecodeFile){
			snprintf(outputfile, MAX_PATH, "%s/%s.wav", outputstrm, sdatfile->strmName.name[i]);
			//STRM open
			nsstrm = nsStrmCreate(sdatfile->strmfile[i].file, sdatfile->strmfile[i].filesize);
			if (!nsstrm) {
				printf("STRM open error.\n");
				continue;
			}
			if(!nsStrmWriteToWaveFile(nsstrm, outputfile)) {
				printf("WAVE write error.\n");
				nsStrmDelete(nsstrm);
				continue;
			}
			else numSTRM++;
			nsStrmDelete(nsstrm);
		} else {
			snprintf(outputfile, MAX_PATH, "%s/%s.strm", outputstrm, sdatfile->strmName.name[i]);
			FILE_outPutFileFromBuff(outputfile, sdatfile->strmfile[i].file, sdatfile->strmfile[i].filesize);
			numSTRM++;
		}
		free(sdatfile->strmfile[i].file);
	}
	
	for (uint32_t i = 0; i < sdatfile->swarName.numFiles; i++) {
		if (sdatfile->swarfile[i].filesize == 0)
			continue;	
		
		//determine filename
		snprintf(outputfile, MAX_PATH, "%s/%s.swar", outputswar, sdatfile->swarName.name[i]);
		//FO_outPutFileFromFP(fp_in, outputfile, sdatfile->swarfile[i].fileoffset, sdatfile->swarfile[i].filesize);
		
		FILE_outPutFileFromBuff(outputfile, sdatfile->swarfile[i].file, sdatfile->swarfile[i].filesize);
		free(sdatfile->swarfile[i].file);
		numSWAR++;
	}

	printf("Total written files:\n");
	printf("    SSEQ:%d\n", numSSEQ);
	printf("    SBNK:%d\n", numSBNK);
	printf("    STRM:%d\n", numSTRM);
	printf("    SWAR:%d\n", numSWAR);
	fclose(fp_in);
}
