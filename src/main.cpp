#include <string>

#include <stdio.h>
#include <getopt.h>

#include "main.h"
#include "sdatxtract.hpp"

//====== global vars =======
bool bDecodeFile = false;

bool bVerboseMessages = false;

bool bExtractSdat = false;

bool bUseFname = true;

bool bGetSwav = false;

const struct option long_options[] = {
	{ "output",        required_argument, NULL, 'o', },
	{ "convert",       no_argument,       NULL, 'c', },
	{ "extract-swar",  no_argument,       NULL, 'x', },
	{ "force-numeric", no_argument,       NULL, 'n', },
	{ "extract-sdat",  no_argument,       NULL, 'S', },
	{ "verbose",       no_argument,       NULL, 'V', },
	{ "help",          no_argument,       NULL, 'h', },
	{ NULL,            no_argument,       NULL,   0, },
};

void printUsage(void) {
	const char *options[] = {
		"", "-c", "output converted files",
		"", "-x", "extract swav from swar",
		"", "-n", "force output with numericaly assigned filenames", 
		"", "-S", "extract sdat only",
		"", "-V", "show verbose messages",
		"", "--help or -h", "show this usage",
	};
	printf("%s v%s\n", APP_NAME, APP_VERSION_FULL);
	printf("Sdat sound archive extraction utility for nds games.\n");
	printf("%s is able to read most sdat files to extract (some) of \nthe audio data they hold.\n\n", APP_NAME);
	printf("Usage: sdatxtract <Options> <Input>\n");
	printf("Options:\n");
	for(long unsigned optIndex = 0; optIndex < sizeof(options) / sizeof(options[optIndex]); optIndex += 3){
		printf("%-2s  %-12s  %s\n", options[optIndex], options[optIndex + 1], options[optIndex + 2]);
	}
}


/* dispatch option char */
bool GET_OptionChar(int opt)
{
	switch(opt)
	{
		case 'c':
			bDecodeFile = true;
			return true;
		case 'x':
			bGetSwav = true;
			return true;
		case 'n':
			bUseFname = false;
			return true;
		case 'h':
			printUsage();
			return true;
		case 'S':
			bExtractSdat = true;
			return true;
		case 'V':
			bVerboseMessages = true;
			return true;

		case ':':
			printf("option needs a value\n");
			return false;
		case '?': //used for some unknown options
			printf("unknown option: %c\n", optopt);
			return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
	if(argc == 1){
		/* no arguments */
		printUsage();
	}
	else{
		int opt;
		while((opt = getopt_long(argc, argv, ":cxnhSV", long_options, NULL)) != -1)
			if (!GET_OptionChar(opt))
				return 1;
	}

	verbose("======================\n");
	verbose("Options: (1=enabled/0=disabled)\n");
	verbose("bDecodeFile:%d\n", bDecodeFile);
	verbose("bGetSwav:%d\n", bGetSwav);
	verbose("bVerboseMessages:%d\n", bVerboseMessages);
	verbose("bUseFname:%d\n", bUseFname);
	verbose("bExtractSdat:%d\n", bExtractSdat);
	verbose("======================\n");

	/* input files */
	for(; optind < argc; optind++) {
		bool isNds = false;

		std::vector<SdatX> sdats = SdatX::Init(argv[optind], isNds);
		if (!sdats.size()) {
			warning("No SDATs found in file: %s\n", argv[optind]);
		} else {
			if (isNds) {
				std::string outdir = fs::path(argv[optind]).stem();
				fs::create_directory(outdir);
				fs::current_path(outdir);
			}

			for (uint32_t i = 0; i < sdats.size(); i++) {
				if (bExtractSdat) {
					if (!sdats[i].Write())
						warning("Failed to write SDAT: %s\n", argv[optind]);
				} else {
					if (!sdats[i].Extract())
						warning("Failed to extract file: %s\n", argv[optind]);
				}
			}

			if (isNds)
				fs::current_path("..");
		}

		sdats.clear();
	}
	return 0;
}
