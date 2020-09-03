#include "sdatxtract.h"
#include "swar.h"
#include "decoder/nsstrm.h"
#include "decoder/nsswav.h"
#include "decoder/sseq2mid.h"

static bool SDATxtract(const char* filepath, nds_t *ndsdata);
static void outputFiles(sdat_t* sdat);

bool extractAudio(const char *filepath) {
	char titleStr[13], codeStr[5];
	char output_dir[MAX_PATH + 1];

	verbose("======================\n");
	verbose("Options: (1=enabled/2=disabled)\n");
	verbose("bDecodeFile:%d\n", bDecodeFile);
	verbose("bVerboseMessages:%d\n", bVerboseMessages);
	verbose("bUseFname:%d\n", bUseFname);
	verbose("======================\n");

	nds_t ndsdata;

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
	FILE_chdir(output_dir);

	if (!SDATxtract(filepath, &ndsdata)) {
		printf("SDAT extraction failed.\n");
	} else {
		printf("SDAT processing complete.\n");
	}

	free(ndsdata.sdatfile);
	FILE_chdir("..");

	return true;
}

bool SDATxtract(const char* filepath, nds_t *ndsdata) {
	char output_dir[5];
	if (filepath == NULL || ndsdata->sdatfile == NULL) {
		return false;
	}

	sdat_t *sdat;
	sdat = malloc(ndsdata->sdatnum*sizeof(sdat_t));

	for (int i = 0; i < ndsdata->sdatnum; i++) {
		if(ndsdata->sdatnum > 1) {
			snprintf(output_dir, sizeof(output_dir), "%d", i);
			FILE_mkdir(output_dir);
			FILE_chdir(output_dir);
		}

		if (bExtractSdat) {
			char out[22];
			snprintf(out, MAX_PATH, "sound_data_%04d.sdat", i);
			printf("Dump Sdat %d\n", i);
			NDS_dumpSDAT(filepath, out, &ndsdata->sdatfile[i]);
		} else {
			printf("Processing Sdat %d\n", i);
			if (SDAT_getFiles(filepath, &ndsdata->sdatfile[i], &sdat[i])) {
				outputFiles(&sdat[i]);
				//SDAT_close(&sdat[i]);
			}
		}

		if(ndsdata->sdatnum > 1) {
			FILE_chdir("..");
		}

		free(ndsdata->sdatfile[i].sdatImage);
	}
	free(sdat);
	return true;
}

void outputFiles(sdat_t* nsdat) {
	char outputfile[MAX_PATH + 1];
	NSSwav *nsswav = NULL;
	NSStrm *nsstrm = NULL;
	Sseq2mid *nssseq = NULL;
	int numSSEQ = 0, numSTRM = 0, numSWAR = 0, numSBNK = 0;
	char tempfname[32];

	FILE_mkdir(DIR_SSEQ);
	FILE_mkdir(DIR_SBNK);
	FILE_mkdir(DIR_STRM);
	FILE_mkdir(DIR_SWAR);

	FILE_chdir(DIR_SSEQ);
	printf("Extracting SSEQ:\n");
	for (uint32_t i = 0; i < nsdat->info.sseq.index.numEntries; i++) {
		if (nsdat->info.sseq.entries[i].fileId == (uint16_t)-1)
			continue;

		fatentry_t *entry = &nsdat->fat.entries[nsdat->info.sseq.entries[i].fileId];
		char* filename = NULL;

		if (bUseFname)
			if (nsdat->symb.sseq.numEntries > i)
				filename = nsdat->symb.sseq.names[i];

		if (!filename) {
			snprintf(tempfname, sizeof(tempfname), "SSEQ_%04d", i);
			filename = tempfname;
		}

		verbose("Prossessing Sseq #%d:  ", i);
		verbose("File %s\n", filename);
		processIndicator();

		if(bDecodeFile){
			snprintf(outputfile, MAX_PATH, "%s.mid", filename);
			nssseq = sseq2midCreate(entry->file, entry->size, false);
			if(!nssseq){
				printf("SSEQ open error.\n");
				continue;
			}
			sseq2midSetLoopCount(nssseq, 1);
			sseq2midNoReverb(nssseq, false);
			if(!sseq2midConvert(nssseq)) {
				printf("SSEQ convert error.\n");
				sseq2midDelete(nssseq);
				continue;
			}
			if(!sseq2midWriteMidiFile(nssseq, outputfile)) {
				printf("MIDI write error.\n");
				sseq2midDelete(nssseq);
				continue;
			}
			else numSSEQ++;
			sseq2midDelete(nssseq);
		} else {
			//determine filename
			snprintf(outputfile, MAX_PATH, "%s.sseq", filename);
			FILE_outPutFileFromBuff(outputfile, entry->file, entry->size);
			numSSEQ++;
		}
	}
	printf("\n");
	FILE_chdir("..");

	FILE_chdir(DIR_SWAR);
	printf("Extracting SWAR:\n");
	for (uint32_t i = 0; i < nsdat->info.swar.index.numEntries; i++) {
		if (nsdat->info.swar.entries[i].fileId == (uint16_t)-1)
			continue;

		fatentry_t *entry = &nsdat->fat.entries[nsdat->info.swar.entries[i].fileId];
		char* filename = NULL;

		if (bUseFname)
			if (nsdat->symb.sseq.numEntries > i)
				filename = nsdat->symb.sseq.names[i];

		if (!filename) {
			snprintf(tempfname, sizeof(tempfname), "SWAR_%04d", i);
			filename = tempfname;
		}

		verbose("Prossessing Swar #%d:    ", i);
		verbose("File %s\n", filename);
		processIndicator();

		if(bDecodeFile || bGetSwav){
			FILE_mkdir(filename);
			FILE_chdir(filename);

			SWAR swar;
			int ret = 0;
			if((ret = SWAREX_init(&swar, entry->file, entry->size))) {
				if (ret == SWARE_BAD)
					printf("SWAR open error: Did not pass validation.\nMay be corrupted?\n");
				if (ret == SWARE_EMPTY)
					printf("SWAR open error: No files found to extract.\n");

				SWAREX_exit(&swar);
				continue;
			}

			for(uint32_t j = 0; j < swar.filenum; j++) {
				SWAV swav;
				verbose("Swav processed %d\n", j);
				if(SWAV_genSwav(&swar.file[j], &swav)){
					printf("SWAV create error.\n");
					SWAV_clear(&swav);
					continue;
				}
				if (bDecodeFile) {
					snprintf(outputfile, MAX_PATH, "%.4d.wav", j);
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
					snprintf(outputfile, MAX_PATH, "%.4d.swav", j);
					FILE_outPutFileFromBuff(outputfile, swav.swavimage, swav.swavsize);
				}
				numSWAR++;
				SWAV_clear(&swav);
			}
			SWAREX_exit(&swar);
			FILE_chdir("..");
		} else {
			snprintf(outputfile, MAX_PATH, "%s.swar", filename);
			FILE_outPutFileFromBuff(outputfile, entry->file, entry->size);
			numSWAR++;
		}
	}
	printf("\n");
	FILE_chdir("..");

	FILE_chdir(DIR_SBNK);
	printf("Extracting SBNK:\n");
	for (uint32_t i = 0; i < nsdat->info.sbnk.index.numEntries; i++) {
		if (nsdat->info.sbnk.entries[i].fileId == (uint16_t)-1)
			continue;

		fatentry_t *entry = &nsdat->fat.entries[nsdat->info.sbnk.entries[i].fileId];
		char* filename = NULL;

		if (bUseFname)
			if (nsdat->symb.sseq.numEntries > i)
				filename = nsdat->symb.sseq.names[i];

		if (!filename) {
			snprintf(tempfname, sizeof(tempfname), "SBNK_%04d", i);
			filename = tempfname;
		}

		verbose("Prossessing Sbnk #%d:   ", i);
		verbose("File %s\n", filename);
		processIndicator();

		//determine filename
		snprintf(outputfile, MAX_PATH, "%s.sbnk", filename);
		FILE_outPutFileFromBuff(outputfile, entry->file, entry->size);
		numSBNK++;
	}
	printf("\n");
	FILE_chdir("..");

	FILE_chdir(DIR_STRM);
	printf("Extracting STRM:\n");
	for (uint32_t i = 0; i < nsdat->info.strm.index.numEntries; i++) {
		if (nsdat->info.strm.entries[i].fileId == (uint16_t)-1)
			continue;

		fatentry_t *entry = &nsdat->fat.entries[nsdat->info.strm.entries[i].fileId];
		char* filename = NULL;

		if (bUseFname)
			if (nsdat->symb.sseq.numEntries > i)
				filename = nsdat->symb.sseq.names[i];

		if (!filename) {
			snprintf(tempfname, sizeof(tempfname), "STRM_%04d", i);
			filename = tempfname;
		}

		verbose("Prossessing Strm #%d:    ", i);
		verbose("File %s\n", filename);
		processIndicator();

		if(bDecodeFile){
			snprintf(outputfile, MAX_PATH, "%s.wav", filename);
			//STRM open
			nsstrm = nsStrmCreate(entry->file, entry->size);
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
			snprintf(outputfile, MAX_PATH, "%s.strm", filename);
			FILE_outPutFileFromBuff(outputfile, entry->file, entry->size);
			numSTRM++;
		}
	}
	printf("\n");
	FILE_chdir("..");

	printf("Total written files:\n");
	printf("    SSEQ: %d\n", numSSEQ);
	printf("    SBNK: %d\n", numSBNK);
	printf("    STRM: %d\n", numSTRM);
	printf("    %s: %d\n", bDecodeFile ? "SWAV" : "SWAR", numSWAR);
}
