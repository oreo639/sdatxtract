#include "sdatxtract.h"

bool SDATxtract(const char* filepath,  const char* outputdir_part1, NDS *ndsdata) {
	bool sucess = false;
	char output_dir[MAX_PATH + 1];
	if (filepath == NULL || ndsdata->ndsfile == NULL) {
		return false;
	}
	
	SDAT sdatfile[ndsdata->sdatnum];
	
	for (int i = 0; i < ndsdata->sdatnum; i++) {
		if(ndsdata->sdatnum > 1) {
			snprintf(output_dir, MAX_PATH, "%s/%d", outputdir_part1, i);
		} else {
			snprintf(output_dir, MAX_PATH, "%s", outputdir_part1);
		}
		
		FILE_mkdir(output_dir);
		
		if (bExtractSdat) {
			char out[MAX_PATH + 1];
			snprintf(out, MAX_PATH, "%s/%d.sdat", outputdir_part1, i);
			printf("Dump Sdat %d\n", i);
			NDS_dumpSDAT(filepath, out, &ndsdata->ndsfile[i]);
		} else {
			printf("Processing Sdat %d\n", i);
			sucess = SDAT_getFiles(filepath, &ndsdata->ndsfile[i], &sdatfile[i]);
			if (sucess == true) {
				SDAT_outputFiles(filepath, output_dir, &sdatfile[i]);
				SDAT_close(&sdatfile[i]);
			}
		}
	}
	return true;
}

bool extractAudio(const char *filepath) {
	char titleStr[13], codeStr[5];
	char output_dir[MAX_PATH + 1];
	
	verbose("======================\n");
	verbose("Options: (1=enabled/2=disabled)\n");
	verbose("bDecodeFile:%d\n", bDecodeFile);
	verbose("bVerboseMessages:%d\n", bVerboseMessages);
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
