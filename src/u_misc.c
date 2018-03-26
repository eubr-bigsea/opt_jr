/*
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <sys/time.h>


#include "u_misc.h"

const double epsilon = 0.001;



double elapsedTime(struct timeval  tv1, struct timeval tv2)
{
	return (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	                     (double) (tv2.tv_sec - tv1.tv_sec);
}


/*
 * 		Name:					doubleCompare
 * 		Input parameters:		double a, double b
 * 		Output parameters:		0  if a = b
 * 								-1 if a < b
 * 								1  if a > b
 * 		Description:			Compare two doubles according to a certain precision (epsilon)
 *
 */

int doubleCompare(double a, double b)
{

	  if (((a - epsilon) < b) &&
	      ((a + epsilon) > b))
	   {
	    return 0;
	   }
	  else
	   {
	    if (a > b) return 1; else return -1;
	   }
}


/*
 * 		Name:					max
 * 		Input parameters:		double a, double b
 * 		Output parameters:		a double
 * 		Description:			It takes the max between two doubles
 *
 */

double max(double a, double b)
{
	if (doubleCompare(a,b) == 1) return(a);
	else return b;
}


/*
 * 		Name:					getCsi
 * 		Input parameters:		double a, double b
 * 		Output parameters:		a double
 * 		Description:			It takes the max between two doubles
 * 		NOTE: This function is possibly redundant
 *
 */

double getCsi(double a, double b)
{
	 if (doubleCompare(a, b) == -1) return b;
	 else return a;
}








/*
 * 		Name:					_run
 * 		Input parameters:		char * cmd
 * 		Output parameters:		The output provided by the executed command
 * 		Description:			This function executes a command ("cmd")
 *
 */

char *_run(char * cmd, struct optJrParameters par)
{

	int BUFSIZE = 10240;
	char debugMsg[DEBUG_MSG];
	int outcome;



	char *buf = (char *)malloc(BUFSIZE);

	if (buf == NULL)
	{
		printf("Malloc failure: _run\n");
		exit(-1);
	}
	    FILE *fp;


	    if ((fp = popen(cmd, "r")) == NULL) {
	        printf("Fatal Error: _run: %s _cmd %s (%d)\n", strerror(errno), cmd, errno);
	        exit(-1);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("OUTPUT: %s", buf);
	    }

	    outcome = pclose(fp);

	    if(outcome == -1)  {
	        printf("Fatal error: Command %s not found or exited with error status\n", cmd);
	        exit(-1);

	    } else sprintf(debugMsg, "_run has returned %d status\n", outcome);debugMessage(debugMsg, par);


	  return buf;
}


struct optJrParameters parseCommandLine(char **args, int argc)
{

	if (argc != ARGS) Usage(argc);


	struct optJrParameters par;

	for (int i = 0; i < ARGS; i++)
		if (strstr(args[i], FILENAME)) strcpy(par.filename, parseArg(args[i], FILENAME, STRING));
		else if (strstr(args[i], NUM_N)) par.number = atoi(parseArg(args[i], NUM_N, NUMBER));
				else if (strstr(args[i], APPLICATIONS_LIMIT)) par.K = atoi(parseArg(args[i], APPLICATIONS_LIMIT, NUMBER));
					else if (strstr(args[i], DEBUG)) par.debug = atoi(parseArg(args[i], DEBUG, YES_NO));
							else if (strstr(args[i], SIMULATOR))
							{
									if (strcmp(parseArg(args[i], FILENAME, STRING), "dagSim") == 0) par.predictor = DAGSIM;
															else par.predictor = LUNDSTROM;
							}
								else if (strstr(args[i], GLOBAL_FO_CALCULATION)) par.globalFOcalculation = atoi(parseArg(args[i], GLOBAL_FO_CALCULATION, YES_NO));
									else if (strstr(args[i], MAX_ITERATIONS))
										par.maxIterations = atoi(parseArg(args[i], MAX_ITERATIONS, NUMBER));
									else if (strstr(args[i], CACHE)) par.cache = atoi(parseArg(args[i], CACHE, YES_NO));
	return par;

}



char * parseArg(char * string, char * gap, int type)
{
	switch(type)
	{
		case NUMBER:
		case STRING:
			return string + strlen(gap);
			break;
		case YES_NO:
			if (strstr(string + strlen(gap), "y") || strstr(string + strlen(gap), "Y")) return "1";
			else if (strstr(string + strlen(gap), "n") || strstr(string + strlen(gap), "N")) return "0";
				else Usage();
			break;
		default:
			printf("Fatal Error: parseArg: option not recognized\n");
			exit(-1);

	}
	return NULL;
}
