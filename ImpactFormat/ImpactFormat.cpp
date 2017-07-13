// ImpactFormat.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <stdio.h>		// printf, fgets
#include <string.h>		// strcmp, strlen
#include <stdlib.h>		// atoi

//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////
#define OP_SUCCESS				0x00
#define OP_FAILURE				0x01


#define FILE_NOFILE				0x10
#define FILE_CORRUPT			0x11
#define FILE_STRUC_ERR			0x12



//////////////////////////////////////////////////////////////////////
// Typedef
//////////////////////////////////////////////////////////////////////
typedef unsigned char  BYTE;
typedef unsigned short WORD;

typedef struct st_format_config
{
	char str_descr[512];
	char str_ver[10];

	BYTE cols;
	BYTE rows;

	char outer;			// y/n
	char header;		// y/n
	char date;			// y/n
	char state;			// y/n

	char main;
	char support;
};

//////////////////////////////////////////////////////////////////////
// service PROCs
//////////////////////////////////////////////////////////////////////
// cut label header from string line
BYTE GetStrTag(char * strParse, char * strOutput, char chOpenSymbol, char chCloseSymbol)
{
	// proc symbols
	// NOTE:
	// cycle proc in next modes:
	// act = 0: cycle end proc   
	// act = 1: try find Open symbol
	// act = 2: try find Close symbol
	char strTag[128];

	BYTE k = 0;
	BYTE ucFstPos = 0;
	BYTE ucEndPos = 0;
	char c;

	BYTE act = 1;
	while (act != 0)
	{
		c = strParse[k];

		// > Check End of Cycle /EOS 
		if (c == '\0')
		{
			// [EOS]

			// define the Reason
			if (act == 1)
			{
				// [NO OPEN SYMBOL FOUND]

				act = 0;

				strOutput = "NULL_START";
				return OP_FAILURE;
			}
			else
			{
				if (act == 2)
				{
					// [NO CLOSE SYMBOL FOUND]

					act = 0;

					strOutput = "NULL_END";
					return OP_FAILURE;
				}
				else
				{
					// [NORMAL OPERATION]

					// NOP: Valid String returned
				}
			}	
		}// > Check End of Cycle /EOS 

		// > Proceed String
		if (act == 1)
		{
			// [proc find Open symbol]

			if (c == chOpenSymbol)
			{
				// [Open symbol Pos Found]

				// define Start Pos
				ucFstPos = k;

				// switch mode 
				act = 2;
			}
		}//then /if (act == 1)
		else
		{
			if (act == 2)
			{
				// [proc find Close symbol]

				if (c == chCloseSymbol)
				{
					// [Close symbol Pos Found]

					// define End Pos
					ucEndPos = k;

					// this was the last mode, so switch to End
					act = 0;
				}
			}//then /if (act == 2)
		}//else /if (act == 1)
		
		// next symbol
		k++;

	}//while (act != 0)

	// SafeCheck
	if (ucFstPos > ucEndPos)
	{
		strOutput = "PARSE_ERROR";
		return OP_FAILURE;
	}

	BYTE ucTagLength = ucEndPos - ucFstPos;

	// > Form Output String /tag/
	for (BYTE kk = 0; kk <= ucTagLength; kk++)
	{
		strOutput[kk] = strParse[ucFstPos + kk];
	}

	strOutput[ucTagLength + 1] = '\0';

	return OP_SUCCESS;
}


// read config file with format parameters
BYTE read_config(st_format_config * Output_format_config)
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
	Output_format_config->str_descr[0] = '\0';
	Output_format_config->str_ver[0] = '\0';

	char str_buf[256] = "";

	// parse strings, sequential
	BYTE parseFlag = 0;
	BYTE paramFlag = 0;

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
			//
			// [descr]			//parseFlag = 1 
			// String Value
			// ...
			// /descr end/
			//
			// [version]		//parseFlag = 2 
			// char[10]
			//
			// [size]			//parseFlag = 3 
			// cols				//paramFlag = 1
			// BYTE
			// rows				//paramFlag = 2
			// BYTE
			//
			// [format]			//parseFlag = 4 
			// outer			//paramFlag = 1
			// char: y/n
			// header			//paramFlag = 2
			// char: y/n
			// date				//paramFlag = 3
			// char: y/n
			// state			//paramFlag = 4
			// char: y/n
			//
			// [symbol]			//parseFlag = 5 
			// main				//paramFlag = 1
			// char
			// support			//paramFlag = 2
			// char

			if (parseFlag == 0)
			{
				// [DEFAULT STATE]

				// check End of Config
				if ((str_buf[0] == '\\') && (str_buf[1] == '\\'))
				{
					// [END OF CONFIG FILE]

					// cancel parse cycle
					act = 0;
				}
				else
				{
					// try to define [lable]
					if (str_buf[0] == '[')
					{
						// [STANDARD LABEL]

						char str_tag[32];

						BYTE readFileProc = GetStrTag(str_buf, str_tag, '[', ']');

						if (readFileProc != OP_SUCCESS)
						{
							// [OP FAILED]

							// > Close File (config) 
							fclose(fs);

							// Exit PROC
							return FILE_STRUC_ERR;
						}

						// define label type
						if (strcmp(str_tag, "[descr]") == 0)
						{
							// set label type
							parseFlag = 1;
						}

						// define label type
						if (strcmp(str_tag, "[version]") == 0)
						{
							// set label type
							parseFlag = 2;
						}

						// define label type
						if (strcmp(str_tag, "[size]") == 0)
						{
							// set label type
							parseFlag = 3;
						}

						// define label type
						if (strcmp(str_tag, "[format]") == 0)
						{
							// set label type
							parseFlag = 4;
						}

						// define label type
						if (strcmp(str_tag, "[symbol]") == 0)
						{
							// set label type
							parseFlag = 5;
						}
					}//if (str_buf[0] == '[')
				}//f ((str_buf[0] == '\\') && (str_buf[0] == '\\'))
			}//then /if (parseFlag == 0)
			else
			{
				// [LABEL STATE]

				// > Parse Label Content
				switch (parseFlag)
				{
				case 1:	// [descr]			
					// check End
					if (strcmp(str_buf, "/descr end/\n") == 0)
					{
						// [END CONDITION]

						// reset parseFlag
						parseFlag = 0;
					}
					else
					{
						// add to description string
						BYTE buf_len = strlen(str_buf);
						BYTE descr_len = strlen(Output_format_config->str_descr);

						BYTE act = 1;

						// safe check
						if (descr_len + buf_len < 512)
						{
							// [VALID]

						}
						else
						{
							// [OVERFLOW]

							act = 0;
						}

						// append to str_descr
						BYTE k = 0;
						char c;

						while (act)
						{
							c = str_buf[k];

							// check EOS
							if (c != '\0')
							{
								// [APPEND]

								Output_format_config->str_descr[descr_len + k] = c;

								// iterator
								k++;
							}
							else
							{
								// [CANCEL]

								// set End of String
								Output_format_config->str_descr[descr_len + k] = '\0';

								act = 0;
							}
						}// while (act)
					}

					break;


				case 2:	// [version]

					// check End of Label
					if (str_buf[0] == '\\')
					{
						// [END OF LABEL]

						// reset parseFlag
						parseFlag = 0;
					}
					else
					{
						// skip Empty Lines
						if (str_buf[0] == '\n')
						{
							// NOP
						}
						else
						{
							// [PROC]

							// check valid length
							BYTE ver_len = strlen(str_buf);
							if (ver_len > 10)
							{
								// [NEED CORRECT]

								str_buf[9] = '\0';
							}
							else
							{
								// [VALID]
							}

							// set to version string
							BYTE k = 0;
							char c;

							BYTE act = 1;
							while (act)
							{
								c = str_buf[k];

								// check EOS
								if (c != '\0')
								{
									// [APPEND]

									Output_format_config->str_ver[k] = c;

									k++;
								}
								else
								{
									// [CANCEL]

									act = 0;
								}
							}// while (act)

							Output_format_config->str_ver[k] = '\0';
						}//else /if (str_buf[0] == '\n')
					}//else /if (str_buf[0] == '\\')

					break;


				case 3:		// [size]

					// check End of Label
					if (str_buf[0] == '\\')
					{
						// [END OF LABEL]

						// reset parseFlag
						parseFlag = 0;
					}
					else
					{
						// skip Empty Lines
						if (str_buf[0] == '\n')
						{
							// NOP
						}
						else
						{
							// [PROC]

							// check paramFlag
							if (paramFlag == 0)
							{
								// [DEFAULT STATE]

								// try to define [param]
								if (strcmp(str_buf, "cols\n") == 0)
								{
									// [PARAM FOUND]

									// set param type
									paramFlag = 1;
								}

								if (strcmp(str_buf, "rows\n") == 0)
								{
									// [PARAM FOUND]

									// set param type
									paramFlag = 2;
								}

							}// then /if (paramFlag == 0)
							else
							{
								// [PARAM STATE]

								BYTE x = 0;

								// > Parse Param Content
								switch (paramFlag)
								{
								case 1:	// cols	

									x = atoi(str_buf);
									Output_format_config->cols = x;

									// reset paramFlag
									paramFlag = 0;

									break;

								case 2:	// rows	

									x = atoi(str_buf);
									Output_format_config->rows = x;

									// reset paramFlag
									paramFlag = 0;

									break;

								default:

									break;
								}//switch (paramFlag)
							}//else /if (paramFlag == 0)						
						}//else /if (str_buf[0] == '\n')
					}//else /if (str_buf[0] == '\\')

					break;


				case 4:		// [format]

					// check End of Label
					if (str_buf[0] == '\\')
					{
						// [END OF LABEL]

						// reset parseFlag
						parseFlag = 0;
					}
					else
					{
						// skip Empty Lines
						if (str_buf[0] == '\n')
						{
							// NOP
						}
						else
						{
							// [PROC]

							// check paramFlag
							if (paramFlag == 0)
							{
								// [DEFAULT STATE]

								// try to define [param]
								if (strcmp(str_buf, "outer\n") == 0)
								{
									// [PARAM FOUND]

									// set param type
									paramFlag = 1;
								}

								if (strcmp(str_buf, "header\n") == 0)
								{
									// [PARAM FOUND]

									// set param type
									paramFlag = 2;
								}

								if (strcmp(str_buf, "date\n") == 0)
								{
									// [PARAM FOUND]

									// set param type
									paramFlag = 3;
								}

								if (strcmp(str_buf, "state\n") == 0)
								{
									// [PARAM FOUND]

									// set param type
									paramFlag = 4;
								}

							}// then /if (paramFlag == 0)
							else
							{
								// [PARAM STATE]

								// > Parse Param Content
								switch (paramFlag)
								{
								case 1:	// outer	

									if ((str_buf[0] == 'y') || (str_buf[0] == 'n'))
									{
										// [VALID]

										Output_format_config->outer = str_buf[0];
									}
									else
									{
										// [INVALID]

										Output_format_config->outer = 'n';
									}

									// reset paramFlag
									paramFlag = 0;

									break;


								case 2:	// header	

									if ((str_buf[0] == 'y') || (str_buf[0] == 'n'))
									{
										// [VALID]

										Output_format_config->header = str_buf[0];
									}
									else
									{
										// [INVALID]

										Output_format_config->header = 'n';
									}

									// reset paramFlag
									paramFlag = 0;

									break;


								case 3:	// date	

									if ((str_buf[0] == 'y') || (str_buf[0] == 'n'))
									{
										// [VALID]

										Output_format_config->date = str_buf[0];
									}
									else
									{
										// [INVALID]

										Output_format_config->date = 'n';
									}

									// reset paramFlag
									paramFlag = 0;

									break;


								case 4:	// state	

									if ((str_buf[0] == 'y') || (str_buf[0] == 'n'))
									{
										// [VALID]

										Output_format_config->state = str_buf[0];
									}
									else
									{
										// [INVALID]

										Output_format_config->state = 'n';
									}

									// reset paramFlag
									paramFlag = 0;

									break;

								default:

									break;
								}//switch (paramFlag)
							}//else /if (paramFlag == 0)						
						}//else /if (str_buf[0] == '\n')
					}//else /if (str_buf[0] == '\\')
					break;


				case 5:		// [symbol]

					// check End of Label
					if (str_buf[0] == '\\')
					{
						// [END OF LABEL]

						// reset parseFlag
						parseFlag = 0;
					}
					else
					{
						// skip Empty Lines
						if (str_buf[0] == '\n')
						{
							// NOP
						}
						else
						{
							// [PROC]

							// check paramFlag
							if (paramFlag == 0)
							{
								// [DEFAULT STATE]

								// try to define [param]
								if (strcmp(str_buf, "main\n") == 0)
								{
									// [PARAM FOUND]

									// set param type
									paramFlag = 1;
								}

								if (strcmp(str_buf, "support\n") == 0)
								{
									// [PARAM FOUND]

									// set param type
									paramFlag = 2;
								}

							}// then /if (paramFlag == 0)
							else
							{
								// [PARAM STATE]

								// > Parse Param Content
								switch (paramFlag)
								{
								case 1:	// main	

									Output_format_config->main = str_buf[0];

									// reset paramFlag
									paramFlag = 0;

									break;


								case 2:	// support	

									Output_format_config->support = str_buf[0];

									// reset paramFlag
									paramFlag = 0;

									break;

								default:

									break;
								}//switch (paramFlag)
							}//else /if (paramFlag == 0)						
						}//else /if (str_buf[0] == '\n')
					}//else /if (str_buf[0] == '\\')

					break;


				default:

					break;

				}//switch (parseFlag)
			}//if (parseFlag == 0)
		}//else if (fgets(str_buf, 256, fs) == "NULL")
	}//then if (fgets(str_buf, 256, fs) == "NULL")	

	// > Close File (config) 
	fclose(fs);


	return OP_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// Main routine
//////////////////////////////////////////////////////////////////////
int main(int argc, char * argv[]) 
{
	// > Output Common Info
	printf("Wellcome to the ImpactFormat! [ver 1.0] \n\n");

	// > Check resource, output status
	st_format_config frm_conf;

	// read config file
	BYTE readFileProc =	read_config(&frm_conf);

	// check file read status
	switch (readFileProc)
	{
	case OP_SUCCESS:
		printf("* Config file read... [OK] \n");

		break;

	case FILE_NOFILE:
		printf("* Config file: No such file in directory. \n");

		break;

	case FILE_STRUC_ERR:
		printf("* Config file: Config file structure error. \n");

		break;		
	default:

		break;

	}//switch (readFileProc)

	printf("\n");


	// > Output Parameters
	printf("Used parameters:\n");
	printf("- executed from: %s\n", argv[0]);

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
	char str_filename[64] = "";

	if (argc > 1)
	{
		// [PARAMS EXISTS]

		// parse params
		for (int k = 0; k < argc; k++)
		{
			// parse k-param String
			if (argv[k][0] == '-')
			{
				// [STANDARD KEY]

				if (strncmp(argv[k], "-help", 5) == 0)
				{
					// print help info
					printf("Help: \n");
				}

				if (strncmp(argv[k], "-config", 7) == 0)
				{
					// print config
					printf("Config: \n");
				}

				if (strncmp(argv[k], "-file", 5) == 0)
				{
					// NOTE:
					// FORMAT: file=filename

					// print file mode params
					char str_filename[64];
					char c;
					BYTE proc_mode = 0;

					BYTE kk = 0;			// cycle foreach argv[k][kk] str
					BYTE k2 = 0;			// cycle foreach str_filename[k2] str

					BYTE act = 1;
					while (act)
					{
						c = argv[k][kk];

						// check End of Param
						if (c == '\0')
						{
							// [PARAM END]

							act = 0;
						}
						else
						{
							// [PROC]

							// > Check parse mode
							if (proc_mode == 0)
							{
								// [INIT]

								// check start of Param Value
								if (c == '=')
								{
									// set mode
									proc_mode = 1;

								}
							}
							else
							{
								if (proc_mode == 1)
								{
									// [FORM FILENAME STRING]

									// append symbol
									str_filename[k2] = c;

									k2++;
								}
							}//else /if (proc_mode == 0)

						}//else /if (c == '\0')

						kk++;

					}

					// put string ender
					str_filename[k2] = '\0';

					printf("%s \n", str_filename);
				}

			}
			else
			{
				// [SPECIFIC KEY]

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

