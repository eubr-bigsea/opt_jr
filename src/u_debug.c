/*
 * u_debug.c
 *
 *  Created on: Oct 31, 2017
 *      Author: work
 */

#include "u_debug.h"

void debugBanner(char * string, struct optJrParameters par)
{
	if (par.debug == YES)
	{
		printf("<debug banner >                  ***************                  *************** \n");
		printf("<debug banner >                  *************** %s *************** \n", string);
		printf("<debug banner >                  ***************                  *************** \n");
	}
}

void debugMessage(char * string, struct optJrParameters par)
{

	if (par.debug == YES)
	printf("<debug message>: %s\n", string);

}


void debugInformational(char * string, struct optJrParameters par)
{
	if (par.debug == YES)
		printf("<debug information >: %s\n", string);
}

void Usage(int argc)
{
		printf("Arguments expected %d, actual arguments number was %d\n", ARGS, argc);
    	printf("Usage:\n");
    	printf("./OPT_JR -f <filename.csv> -n <N> -k <Limit> -d <dY/y|N/n> -c <Y/y|N/n> -s <dagSim|lundstrom> -i <iterations>\n");
    	printf("where:\n");
    	printf("<filename.csv> is the csv file (including the input values) under $UPLOAD_HOME in wsi_config.xml;\n");
    	printf("<N> is the total number of cores;\n");
    	printf("<Limit> is the maximum number of considered candidates (if equal to 0, all the candidates are considered).\n");
    	printf("-d represents debug (on/off)\n");
    	printf("-c represents cache (on/off)\n");
    	printf("-i represents the maximum number of iterations\n");
    	printf("Example:\n");
    	printf("./OPT_JR -f=\"Test3.csv\" -n=220 -k=0 -d=Y -s=dagSim -i=10\n");
    	printf("OPT_JR is executed on a file Test3.csv, the total number of cores is 220, all the candidates are considered, the predictor used is dagSim, "
    			"the maximum number of iterations is 10\n");
    	exit(-1);
    }

