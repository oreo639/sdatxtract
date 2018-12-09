#include "sdatxtract.h"

//====== global vars =======
bool bDecodeFile = false;

bool bVerboseMessages = false;

bool bExtractSdat = false;

void printUsage(void) {
	const char *options[] = {
		"", "-c", "decode files", 
		"", "-S", "extract sdat only",
		"", "-V", "show verbose messages",
		"", "--help or -h", "show this usage",
	};
	printf("%s v%s\n", APP_NAME, APP_VERSION_FULL);
	printf("Sdat sound archive extraction utility for nds games.\n");
	printf("%s is able to read most nds games and sdat files to extract (some) of \nthe audio data they hold.\n\n", APP_NAME);
	printf("Usage: sdatxtract(.exe) <Options> <Input>\n"); 
	printf("Options:\n");
	for(int optIndex = 0; optIndex < sizeof(options) / sizeof(options[optIndex]); optIndex += 3){
		printf("%-2s  %-12s  %s\n", options[optIndex], options[optIndex + 1], options[optIndex + 2]);
	}
}


/* dispatch option char */
bool GET_OptionChar(const char opt)
{
	switch(opt)
	{
	case 'c':
		bDecodeFile = true;
		return true;
	case 'S':
		bExtractSdat = true;
		return true;
	case 'h':
		printUsage();
		return true;
	case 'V':
		bVerboseMessages = true;
		return true;
	}
	return false;
}

/* dispatch option string */
bool GET_OptionStr(const char* optStr)
{
	if(strcmp(optStr, "help") == 0){
		printUsage();
	}
	else{
		return false;
	}
	return true;
}

int main(int argc, char* argv[])
{
	int argi = 1;
	int argci;

	if(argc == 1){
		/* no arguments */
		printUsage();
	}
	else{
		/* options */
		while((argi < argc) && (argv[argi][0] == '-'))
		{
			if(argv[argi][1] == '-'){
				/* --string */
				if(!GET_OptionStr(&argv[argi][2])){
					printf("Invalid option.\n");
					return 1;
				}
			}
			else{
				/* -letters */
				argci = 1;
				while(argv[argi][argci] != '\0')
				{
					if(!GET_OptionChar(argv[argi][argci])){
						printf("Invalid option.\n");
						return 1;
					}
					argci ++;
				}
			}
			argi++;
		}
	}

	/* input files */
	for(; argi < argc; argi ++){
		extractAudio(argv[argi]);
	}
	return 0;
}
