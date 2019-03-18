#include "sdatxtract.h"
#include "swar.h"
#include "decoder/nsstrm.h"
#include "decoder/nsswav.h"
#include "decoder/sseq2mid.h"

bool extractAudio(const char *filepath) {
	char titleStr[13], codeStr[5];
	char output_dir[MAX_PATH + 1];

	verbose("======================\n");
	verbose("Options: (1=enabled/2=disabled)\n");
	verbose("bDecodeFile:%d\n", bDecodeFile);
	verbose("bVerboseMessages:%d\n", bVerboseMessages);
	verbose("bUseFname:%d\n", bUseFname);
	verbose("======================\n");

	NDS ndsdata;

	if (SDAT_isSDAT(filepath)) {
		snprintf(titleStr, 13, "SDAT");
		if (!SDAT_getUniqueId(filepath, codeStr)) {
			printf("Failed to open SDAT.\n");
			return false;
		}

		if (!SDAT_fakeNds(filepath, &ndsdata)) {
			printf("Failed to read SDAT.\n");
			return false;
		}
	}
	else {
		if (!NDS_getGameTitle(filepath, titleStr)) {
			printf("Failed to open NDS.\n");
			return false;
		}
		if (!NDS_getGameCode(filepath, codeStr)) {
			printf("Failed to open NDS.\n");
			return false;
		}
		if (!NDS_isNds(filepath)) {
			printf("[WARN]:The file does not appear to be an nds file or an sdat file. The extraction will continue anyways.\n");
		}

		if (!NDS_getSDAToffset(filepath, &ndsdata)) {
			printf("No valid SDAT found.\n");
			return false;
		}
	}
	printf("TITLE  : \"%s\"\n", titleStr);
	printf("CODE  : \"%s\"\n", codeStr);
	snprintf(output_dir, MAX_PATH, "%s_%s", titleStr, codeStr);
	verbose("Outputing to directory: %s\n", output_dir);
	FILE_mkdir(output_dir);

	if (!SDATxtract(filepath, output_dir, &ndsdata)) {
		printf("SDAT extraction failed.\n");
	} else {
		printf("SDAT processing complete.\n");
	}

	free(ndsdata.ndsfile);
	return true;
}

bool SDATxtract(const char* filepath,  const char* outputdir_part1, NDS *ndsdata) {
	bool sucess = false;
	char output_dir[MAX_PATH + 1];
	if (filepath == NULL || ndsdata->ndsfile == NULL) {
		return false;
	}

	SDAT *sdatfile;
	sdatfile = malloc(ndsdata->sdatnum*sizeof(SDAT));

	for (int i = 0; i < ndsdata->sdatnum; i++) {
		if(ndsdata->sdatnum > 1) {
			snprintf(output_dir, MAX_PATH, "%s/%d", outputdir_part1, i);
		} else {
			snprintf(output_dir, MAX_PATH, "%s", outputdir_part1);
		}

		FILE_mkdir(output_dir);

		if (bExtractSdat) {
			char out[MAX_PATH + 1];
			snprintf(out, MAX_PATH, "%s/sound_data_%04d.sdat", outputdir_part1, i);
			printf("Dump Sdat %d\n", i);
			NDS_dumpSDAT(filepath, out, &ndsdata->ndsfile[i]);
		} else {
			printf("Processing Sdat %d\n", i);
			sucess = SDAT_getFiles(filepath, &ndsdata->ndsfile[i], &sdatfile[i]);
			if (sucess == true) {
				outputFiles(output_dir, &sdatfile[i]);
				SDAT_close(&sdatfile[i]);
			}
		}
		free(ndsdata->ndsfile[i].sdatImage);
	}
	free(sdatfile);
	return true;
}

void outputFiles(const char* outputdir_part1, SDAT* sdatfile) {
	char outputfile[MAX_PATH + 1], outputsseq[MAX_PATH - 34], outputsbnk[MAX_PATH - 34], outputstrm[MAX_PATH - 34], outputswar[MAX_PATH - 34];
	NSSwav *nsswav = NULL;
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

		verbose("Prossessing Sseq #%d:  ", i);
		verbose("File %s\n", sdatfile->sseqName.name[i]);

		if(bDecodeFile){
			snprintf(outputfile, MAX_PATH, "%s/%s.mid", outputsseq, sdatfile->sseqName.name[i]);
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
			sseq2midDelete(nssseq);
		} else {
			//determine filename
			snprintf(outputfile, MAX_PATH, "%s/%s.sseq", outputsseq, sdatfile->sseqName.name[i]);
			FILE_outPutFileFromBuff(outputfile, sdatfile->sseqfile[i].file, sdatfile->sseqfile[i].filesize);
			numSSEQ++;
		}
	}

	for (uint32_t i = 0; i < sdatfile->sbnkName.numFiles; i++) {
		if (sdatfile->sbnkfile[i].filesize == 0)
			continue;

		verbose("Prossessing Sbnk #%d:   ", i);
		verbose("File %s\n", sdatfile->sbnkName.name[i]);

		//determine filename
		snprintf(outputfile, MAX_PATH, "%s/%s.sbnk", outputsbnk, sdatfile->sbnkName.name[i]);
		FILE_outPutFileFromBuff(outputfile, sdatfile->sbnkfile[i].file, sdatfile->sbnkfile[i].filesize);
		numSBNK++;
	}

	for (uint32_t i = 0; i < sdatfile->strmName.numFiles; i++) {
		if (sdatfile->strmfile[i].filesize == 0)
			continue;

		verbose("Prossessing Strm #%d:    ", i);
		verbose("File %s\n", sdatfile->strmName.name[i]);

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
	}

	for (uint32_t i = 0; i < sdatfile->swarName.numFiles; i++) {
		if (sdatfile->swarfile[i].filesize == 0)
			continue;
		verbose("Prossessing Swar #%d:    ", i);
		verbose("File %s\n", sdatfile->swarName.name[i]);
		if(bDecodeFile || bGetSwav){
			SWAR swar;
			if(SWAREX_init(&swar, sdatfile->swarfile[i].file, sdatfile->swarfile[i].filesize) ){
				printf("SWAR open error.\n");
				SWAREX_exit(&swar);
				continue;
			}

			for(uint32_t j = 0; j < swar.filenum; j ++) {
				SWAV swav;
				verbose("Swav processed %d\n", j);
				if(SWAV_genSwav(&swar.file[j], &swav)){
					printf("SWAV create error.\n");
					SWAV_clear(&swav);
					continue;
				}
				if (bDecodeFile) {
					snprintf(outputfile, MAX_PATH, "%s/%s_%.4d.wav", outputswar, sdatfile->swarName.name[i], j);
					nsswav = nsSwavCreate(swav.swavimage, swav.swavsize);
					if(!nsswav){
						printf("SWAV open error.\n");
						SWAV_clear(&swav);
						continue;
					}

					if(!nsSwavWriteToWaveFile(nsswav, outputfile)){
						printf("WAVE write error.\n");
						nsSwavDelete(nsswav);
						SWAV_clear(&swav);
						continue;
					}
					nsSwavDelete(nsswav);
				} else {
					snprintf(outputfile, MAX_PATH, "%s/%s_%.4d.swav", outputswar, sdatfile->swarName.name[i], j);
					FILE_outPutFileFromBuff(outputfile, swav.swavimage, swav.swavsize);
				}
				numSWAR++;
				SWAV_clear(&swav);
			}
			SWAREX_exit(&swar);
		} else {
			snprintf(outputfile, MAX_PATH, "%s/%s.swar", outputswar, sdatfile->swarName.name[i]);
			FILE_outPutFileFromBuff(outputfile, sdatfile->swarfile[i].file, sdatfile->swarfile[i].filesize);
			numSWAR++;
		}
	}

	printf("Total written files:\n");
	printf("    SSEQ:%d\n", numSSEQ);
	printf("    SBNK:%d\n", numSBNK);
	printf("    STRM:%d\n", numSTRM);
	printf("    SWAR:%d\n", numSWAR);
}
