// ImpactFormat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////
#define OP_SUCCESS				0x00

#define FILE_NOFILE				0x10
#define FILE_CORRUPT			0x11

//////////////////////////////////////////////////////////////////////
// Typedef
//////////////////////////////////////////////////////////////////////
typedef unsigned char BYTE;

//////////////////////////////////////////////////////////////////////
// service PROCs
//////////////////////////////////////////////////////////////////////
// cut label header from string line
char * GetStrLabel(char * strParse, char chOpenSymbol, char chCloseSymbol)
{
	// proc symbols
	// NOTE:
	// cycle proc in next modes:
	// act = 0: cycle end proc   
	// act = 1: try find Open symbol
	// act = 2: try find Close symbol
	BYTE k = 0;
	char c;

	BYTE act = 1;
	while (act != 0)
	{
		c = strParse[k];

		// check End of String 
		if (c == '\0')
		{
			if (act == 1)
			{
				// [NO OPEN SYMBOL FOUND]

				act = 0;

				return "NULL_START";
			}
			else
			{
				if (act == 2)
				{
					// [NO CLOSE SYMBOL FOUND]

					act = 0;

					return "NULL_END";
				}
				else
				{

				}
			}

			
		}

		if (act == 1)
		{
			// [find Open symbol]

			if (c == chOpenSymbol)
			{
				// [Open symbol Pos Found]


				// switch mode 
				act = 2;
			}
		}//then /if (act == 1)
		else
		{
			if (act == 2)
			{
				// [find Close symbol]

				// this was the last mode, so switch to End
				act = 0;
			}
		}//else /if (act == 1)
		

	}//while (act != 0)

	return "NULL";
}


// read config file with format parameters
BYTE read_config()
{
	// > Open File (config)
	// default name
	char* file_name = "config.txt";

	// try open
	FILE *fs = fopen(file_name, "r");

	// check OP result
	if (fs == NULL)
	{
		// [fail to open]

		return FILE_NOFILE;
	}

	// > Read config
	char str_buf[256];

	// parse strings, sequential
	BYTE parseFlag = 0;

	BYTE act = 1;
	while (act)
	{
		// get File String Line
		if (fgets(str_buf, 256, fs) == "NULL")
		{
			// [STOP]

			act = 1;

			// check the Reason
			if (feof(fs))
			{
				// [EOF]

				// Valid case
			}
			else
			{
				if (ferror(fs))
				{
					// [ERROR]
				}
			}//NULL Read reason
		}
		else
		{
			// [PROCEED]

			// NOTE: File (config) has specific format:
			// FORMAT:
			// file has variour [label]s. 
			// Each [label] include some params.
			// Each param has some data type Value STRICTLY on the NEXT Line.
			// After Value '\n' divider may be exist.

			// [descr]			//parseFlag = 1 
			// String Value
			// ...
			// /descr end/

			// [version]		//parseFlag = 2 
			// char[10]

			// [size]			//parseFlag = 3 
			// cols
			// BYTE
			// rows
			// BYTE

			// [format]			//parseFlag = 4 
			// outer
			// char: y/n
			// header
			// char: y/n
			// date
			// char: y/n
			// state
			// char: y/n
			
			// [symbol]			//parseFlag = 5 
			// main
			// char
			// support
			// char

			if (parseFlag == 0)
			{
				// [DEFAULT STATE]

				// try to define [lable]
				if (str_buf[0] == '[')
				{
					// [STANDARD LABEL]

					char * pEndSign = strchr(str_buf, ']');
					if (pEndSign != "NULL")
					{
						
					}

					// define label type
					if (str_buf == "[descr]")
					{
						// set label type
						parseFlag = 1;
					}


				}// try to define [lable]

			}//if (parseFlag == 0)


		}//else if (fgets(str_buf, 256, fs) == "NULL")


	}//then if (fgets(str_buf, 256, fs) == "NULL")

	

	// > Close File (config) 
	fclose(fs);

}

//////////////////////////////////////////////////////////////////////
// Main routine
//////////////////////////////////////////////////////////////////////
int main(int argc, char * argv[]) 
{
	// > Output Common Info


	// > Check resource, output status
	BYTE readFileProc =	read_config();

	// check file open result
	switch (readFileProc)
	{
	case OP_SUCCESS:

		break;

	case FILE_NOFILE:
		printf("No such file in directory. \n");

		break;

	default:

		break;

	}//switch (readFileProc)
	

	// > Output Parameters
	printf("used parameters:\n");
	printf("# executed from: %s\n", argv[0]);

	if (argc > 1)
	{
		// [PARAMS EXISTS]
		// output Values
		for (int k = 0; k < argc; k++)
		{
			printf("- parameter %d: %s\n", k, argv[k]);
		}
	}
	else
	{
		// [NO PARAMS]
		// output Status
		printf("[command line has no additional arguments]\n");
	}

	// > Parse parameters:
	if (argc > 1)
	{
		// [PARAMS EXISTS]
		// parse params
		for (int k = 0; k < argc; k++)
		{
			// parse k-param String
			BYTE act = 1;
			while (act)
			{
				if (argv[k][0] == '-')
				{
					// [STANDARD KEY]

					if (argv[k] == "-help")
					{
						// print help info
						printf("Help: \n");
					}

					if (argv[k] == "-c")
					{
						// print config
						printf("Config: \n");
					}

				}
				else
				{
					// [SPECIFIC KEY]
				}
			}// parse k-param

		}
	}
	

	// > End of program
	// wait any key press
	getchar();

    return 0;


	/*
	// > additional function
	char c1[20];
	gets_s(c1, 20);

	puts(c1);

	*/

}

