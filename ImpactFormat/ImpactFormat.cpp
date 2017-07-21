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

#define MAX_STR_BUF				256
#define MAX_STR_TIT				64
#define MAX_STR_DESCR			512
#define MAX_STR_VER				10
#define MAX_STR_TAG				128
#define MAX_STR_LINE			64

//////////////////////////////////////////////////////////////////////
// Typedef
//////////////////////////////////////////////////////////////////////
typedef unsigned char  BYTE;
typedef unsigned short WORD;

typedef struct st_format_config
{
	char str_descr[MAX_STR_DESCR];
	char str_ver[MAX_STR_VER];

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
	char strTag[MAX_STR_TAG];

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

// fill str with defined char from position X to X + C
BYTE Fill_Char(char * str_Fill, char chSymbol, BYTE ucFromPos, BYTE ucCount)
{
	// fill OP
	for (BYTE k = 0; k < ucCount; k++)
	{
		str_Fill[ucFromPos + k] = chSymbol;
	}

	// add string ender
	str_Fill[ucFromPos + ucCount] = '\0';

	return 0;
}

// copy number C of chars from str_input position X1 to str_output position X2
BYTE Append_StrPart(char * str_Input, char * str_Output, BYTE ucFromPos, BYTE ucToPos, BYTE ucCount)
{
	// copy OP
	for (BYTE k = 0; k < ucCount; k++)
	{
		str_Output[ucToPos + k] = str_Input[ucFromPos + k];
	}

	// transfer string ender
	str_Output[ucToPos + ucCount] = '\0';

	return 0;
}

// read config file with format parameters
BYTE Read_config(st_format_config * Output_format_config)
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

	char str_buf[MAX_STR_BUF] = "";

	// parse strings, sequential
	BYTE parseFlag = 0;
	BYTE paramFlag = 0;

	BYTE act = 1;
	while (act)
	{
		// get File String Line
		if (fgets(str_buf, MAX_STR_BUF, fs) == "NULL")
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

						char str_tag[MAX_STR_TAG];

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
						if (descr_len + buf_len < MAX_STR_DESCR)
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
							if (ver_len > MAX_STR_VER)
							{
								// [NEED CORRECT]

								str_buf[MAX_STR_VER - 1] = '\0';
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


// read config file with format parameters
BYTE Interpret_impact(char * str_imputFilename, char * str_outputFilename, st_format_config Output_format_config)
{
	// > Define RowTypes Matrix

	// FORMAT:
	// # Row Attributes:
	// VAL | TYPE
	//  0  | SMARK
	//  1  | FMARK
	//  2  | XBAR
	//  3  | YBAR
	//  4  | NBAR
	//  5  | TITLE
	//  6  | STATE
	//  7  | TEXT
	//
	// # Row	Types
	// NUM |	
	//  0  |	|===========			========| 
	//  1  |	|	# TITLE				/DATE	|
	//  2  |	|-----------			--------|
	//  3  |	|	/null string/  		        |
	//  k  |	|	TEXT       			        |
	// N-4 |	|	/null string/  		        |
	// N-3 |	|-----------			--------|
	// N-2 |	| 						/PAGE X |
	// N-1 |	|===========			========| 

	BYTE v_rowTypes[9][8] = {
//	0  1  2  3  4  5  6  7 
	1, 1, 1, 0, 0, 0, 0, 0,	// [0]
	1, 1, 0, 0, 0, 1, 0, 0,	// [1]
	1, 1, 0, 1, 0, 0, 0, 0,	// [2]
	1, 1, 0, 0, 1, 0, 0, 0,	// [3]
							// ...
	1, 1, 0, 0, 0, 0, 0, 1,	// [k]
							// ...
	1, 1, 0, 0, 1, 0, 0, 0,	// [N-4]
	1, 1, 0, 1, 0, 0, 0, 0,	// [N-3]
	1, 1, 0, 0, 0, 0, 1, 0,	// [N-2]
	1, 1, 1, 0, 0, 0, 0, 0,	// [N-1]
	};

	const BYTE RT_TEMPL_S = 4;
	const BYTE RT_TEMPL_F = 4;
	const BYTE RT_TEMPL_N = 9;


	// > File init OPs
	// open Text File
	// // try open
	FILE *ft = fopen(str_imputFilename, "r");

	// // check OP result
	if (ft == NULL)
	{
		// [fail to open]

		return FILE_NOFILE;
	}

	// > Get TITLE
	// Line 00: TITLE
	char str_buf[MAX_STR_BUF] = "";

	// get File Text Line
	if (fgets(str_buf, MAX_STR_TIT, ft) == "NULL")
	{
		// [fail to open]

		return FILE_STRUC_ERR;
	}

	// set Title Text
	char str_TITLE[MAX_STR_TIT];
	strncpy(str_TITLE, str_buf, MAX_STR_TIT);

	// > Create an empty File for output impact formatted. 
	FILE *fi = fopen(str_outputFilename, "w");

	// > Row Type Get routine	[#01]
	// FORMAT:
	// Text File content type:
	// Line 00: TITLE
	// Line XX: TEXT

	BYTE k = 0;				// Impact Row 
	BYTE ucRowType;
	BYTE ucLinePos;
	BYTE ucLineSeg;			// number of impact segmented line
	BYTE ucPageSeg = 0;		// number of impact segmented page
	BYTE bLineCont = 0;		// file line = impact multiline
	BYTE bFileCont = 1;		// impact end line reach

	char str_Line[MAX_STR_LINE];

	BYTE act = 1;
	while (act)
	{
		// check k
		if (k < Output_format_config.rows)
		{
			// [IN IMPACT PAGE]

			if (k < RT_TEMPL_S)
			{
				// [IN IMPACT PAGE STARTER]

				ucRowType = k;
			}
			else
			{
				if (k > Output_format_config.rows - RT_TEMPL_F - 1)
				{
					// [IN IMPACT PAGE FINALIZES]

					ucRowType = RT_TEMPL_N - (Output_format_config.rows - k);
				}
				else
				{
					// [IN IMPACT PAGE TEXT]

					// position next to RT_TEMPL_S is TEXT position
					ucRowType = RT_TEMPL_S;
				}
			}//else /if (k < RT_TEMPL_S)

			// next Impact Line
			k++;

		}//then /if (k < Output_format_config.rows)
		else
		{
			// [IMPACT PAGE ENDED]

			if (bFileCont)
			{
				// [TEXT FILE NOT ENDED]

				// reset impact page Row counter
				k = 0;

				// define RowType
				ucRowType = k;
			}
			else
			{
				// [TEXT FILE ENDS]

				// end of formatting Proc
				act = 0;
			}
		}//else /if (k < Output_format_config.rows)

		// > Fill Row routine [#02]
		// reset Line Position
		ucLinePos = 0;
		
		// reset Line String
		str_Line[0] = '\0';

		// proceed RowType
		// NOTE: sequence is strictly recommended

		// [%]
		if (v_rowTypes[ucRowType][0])
		{
			// [START_MARK]

			// !need correct
			// str_Line[ucLinePos] = Output_format_config.main;
			str_Line[ucLinePos] = '|';

			ucLinePos++;
		}

		// [%]
		if (v_rowTypes[ucRowType][2])
		{
			// [CROSS_BAR]

			Fill_Char(str_Line, Output_format_config.main, 1, Output_format_config.cols - 2);

			ucLinePos += Output_format_config.cols - 2;
		}

		// [%]
		if (v_rowTypes[ucRowType][3])
		{
			// [SUPPORT_BAR]

			Fill_Char(str_Line, Output_format_config.support, 1, Output_format_config.cols - 2);

			ucLinePos += Output_format_config.cols - 2;
		}

		// [%]
		if (v_rowTypes[ucRowType][4])
		{
			// [NULL_BAR]

			//!need to replace with STATE
			Fill_Char(str_Line, ' ', 1, Output_format_config.cols - 2);

			ucLinePos += Output_format_config.cols - 2;

		}

		// [%]
		if (v_rowTypes[ucRowType][5])
		{
			// [TITLE]

			//!need to replace with STATE
			Fill_Char(str_Line, ' ', 1, Output_format_config.cols - 2);

			ucLinePos += Output_format_config.cols - 2;

		}

		// [%]
		if (v_rowTypes[ucRowType][6])
		{
			// [STATE]

			//!need to replace with STATE
			Fill_Char(str_Line, ' ', 1, Output_format_config.cols - 2);

			ucLinePos += Output_format_config.cols - 2;
		}

		// [%]
		if (v_rowTypes[ucRowType][7])
		{
			// [TEXT]

			if (bFileCont)
			{
				// [FILE TEXT PROC]

				// check file continuation
				if (bLineCont == 0)
				{
					// [STRING LINE START]

					// get File Text Line
					if (fgets(str_buf, MAX_STR_BUF, ft) == "NULL")
					{
						// [END OF FILE]

						// Text file ends
						bFileCont = 0;

						// set buffer Value to end text OP
						str_buf[0] = '\0';
					}
					else
					{
						// [CONTINUE TEXT]

						// init segment number
						ucLineSeg = 0;

					}//if (fgets(str_buf, MAX_STR_BUF, ft) == "NULL")
				}//then /if (bCont = 0)
				else
				{
					// [STRING LINE CONTINUE]

					// NOP

				}//else /if (bLineCont = 0)

				// > Proceed buffer with text file line 
				BYTE ucLineTextSize = strlen(str_buf);
				BYTE ucLineSize = 0;

				if ( ucLineTextSize < (ucLineSeg + 1) * (Output_format_config.cols - 2) )
				{
					// [LINE TERMINATES]

					// calc Line String Remains
					ucLineSize = ucLineTextSize - ucLineSeg * (Output_format_config.cols - 2);

					// copy Text from Line
					Append_StrPart(str_buf, str_Line, ucLineSeg * (Output_format_config.cols - 2), ucLinePos, ucLineSize);

					// update current LinePos
					ucLinePos += ucLineSize;

					// fill Null-spaces to the End of Impact Line
					BYTE ucLineRemains = Output_format_config.cols - 2 - ucLineSize;
					Fill_Char(str_Line, ' ', ucLinePos, ucLineRemains);

					// update current LinePos
					ucLinePos += ucLineRemains;

				}
				else
				{
					// [LINE CONTINUATES]

					// calc Line String Remains
					ucLineSize = Output_format_config.cols - 2;

					// copy Text from Line
					Append_StrPart(str_buf, str_Line, ucLineSeg * (Output_format_config.cols - 2), ucLinePos, ucLineSize);

					// update current LinePos
					ucLinePos += ucLineSize;

					// set Continue Flag
					bLineCont = 1;

				}//else /if ( ucLineTextSize < (ucLineSeg + 1) * (Output_format_config.cols - 2) )

				// inc segment number
				ucLineSeg++;

			}//then /if (bFileCont)
			else
			{
				// [FILE TEXT ENDS]
				// fill nulls to the end of impact page

				// null string
				str_buf[0] = '\0';

			}//else /if (bFileCont)
		}//if (v_rowTypes[ucRowType, 6])

		// [%]
		if (v_rowTypes[ucRowType][1])
		{
			// [FINAL_MARK]

			// !need correct
			// str_Line[ucLinePos] = Output_format_config.main;
			str_Line[ucLinePos] = '|';

			// update current LinePos
			ucLinePos++;
		}

		// set String Line ender
		str_Line[ucLinePos] = '\0';

		// > Write Line to Impact Formatted File
		fputs(str_Line, fi);

	}//while (act)


	// > Close File (text) 
	fclose(ft);

	// > Close File (impact) 
	fclose(fi);


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
	BYTE readFileProc =	Read_config(&frm_conf);

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

				if (strncmp(argv[k], "-s", 2) == 0)
				{
					// print help info
					printf("silent mode: \n");
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
				//[SPECIFIC KEY]

			}//parse k-param
		}//for (int k = 0; k < argc; k++)
	}//if (argc > 1)
	
	// > Print formatted routine
	Interpret_impact("textfile.txt", "textfile_out.txt", frm_conf);
	


	// > End of program
	// wait any key press
	printf("\n %s \n", "Press any key to exit...");
	getchar();

    return 0;


	/*
	// > additional function
	char c1[20];
	gets_s(c1, 20);

	puts(c1);

	*/

}

