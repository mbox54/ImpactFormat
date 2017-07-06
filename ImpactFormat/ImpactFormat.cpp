// ImpactFormat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// Typedef
typedef unsigned char BYTE;

// service PROCs
// read config file with format parameters
BYTE read_config()
{
	char* file_name = "config.txt";

}


int main(int argc, char * argv[]) 
{
	// > Output Common Info

	// > Check resource, output status


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

