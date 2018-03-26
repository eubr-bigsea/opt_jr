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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "common.h"
#include "main.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int main(int argc, char **argv)
{
    struct optJrParameters par;
    char debugMsg[DEBUG_MSG];
    struct timeval  tv_initial_main,
						tv_initial_bounds,
	        			tv_final_bounds,
        				tv_initial_nu,
        				tv_final_nu,
    					tv_initial_init,
    					tv_final_init,
        				tv_initial_fix,
        				tv_final_fix,
        				tv_initial_locals,
        				tv_final_locals,
        				tv_final_main;

    /* Parse the command line */
    par = parseCommandLine(argv, argc);

    /* Upload the configuration file ($HOME/wsi_config.xml) into a list */
    sConfiguration *configuration = readConfigurationFile();

    /* Read the threads number */
    par.numberOfThreads = atoi(getConfigurationValue(configuration, "OPTIMIZE_NUM_PROCESSES"));

    /* Connect to the db */
    MYSQL *conn = DBopen(
            			getConfigurationValue(configuration, "DB_IP"),
						getConfigurationValue(configuration, "DB_port"),
    					getConfigurationValue(configuration, "DB_user"),
    					getConfigurationValue(configuration, "DB_pass"),
    					getConfigurationValue(configuration, "DB_dbName")
    					);
    if (conn == NULL) DBerror(conn, "open_db: Opening the database");

    // Calculate the time taken
    gettimeofday(&tv_initial_main, NULL);

    /* Load applications details from csv file */
    sApplication *first = parseCsv(configuration, par);
    /*sApplication *current = NULL;
    sApplication *first = NULL;
    addApplication(par, configuration, &first, &current, "application_1483347394756_0","query26",1, 18906.97517,12945621.49, 28,8,4,2,200000,3.5, "J4S5",500);*/

    printApplicationsParameters(first, par);

    /* Calculate the bounds */
    gettimeofday(&tv_initial_bounds, NULL);
    if (par.numberOfThreads == 0) calculateBounds(first, configuration, conn, par);
       	else calculateOpenMPBounds(first, par.numberOfThreads, configuration, conn, par);
    printApplications(first, par);
    gettimeofday(&tv_final_bounds, NULL);


    /* Calculate the indices */
    gettimeofday(&tv_initial_nu, NULL);
    calculate_Nu(configuration, conn, first,  par);printApplications(first, par);
    checkTotalNodes(par.number, first, par);
    gettimeofday(&tv_final_nu, NULL);


     /* Fix initial solution */
     gettimeofday(&tv_initial_fix, NULL);
     sApplicationPointers *firstPointer = fixInitialSolution(first, par);
     gettimeofday(&tv_final_fix, NULL);
     printApplications(first, par);

    /* Calculate baseFO for each application */
    gettimeofday(&tv_initial_init, NULL);
    initialize(configuration, conn, first, par);
    gettimeofday(&tv_final_init, NULL);


    /* Invoke localSearch */
    gettimeofday(&tv_initial_locals, NULL);
    localSearch(configuration, conn, first, par);
    gettimeofday(&tv_final_locals, NULL);

    debugInformational("Final solution\n", par);
    writeResults(conn, getConfigurationValue(configuration,"DB_dbName"),first, par);

    /* De-allocate resources and close connection */
    freeParameters(first);
    freeApplications(firstPointer);

    DBclose(conn);

    gettimeofday(&tv_final_main, NULL);
    sprintf(debugMsg, "FixInitial step elapsed time: %lf\n", elapsedTime(tv_initial_fix, tv_final_fix));printf(debugMsg);
    sprintf(debugMsg, "Findbounds  elapsed time: %lf\n", elapsedTime(tv_initial_bounds, tv_final_bounds));printf(debugMsg);
    sprintf(debugMsg, "Initialization elapsed time %lf\n", elapsedTime(tv_initial_nu, tv_final_nu));printf(debugMsg);
    sprintf(debugMsg, "LocalSearch step elapsed time: %lf\n", elapsedTime(tv_initial_locals, tv_final_locals));printf(debugMsg);
    sprintf(debugMsg, "Overall elapsed time: %lf\n", elapsedTime(tv_initial_main, tv_final_main));printf(debugMsg);


    //printOutput(first);
    sprintf(debugMsg, "This run was executed with the following configuration:\n "
    		"file: %s, cache enabled: %d, max iterations: %d debug info: %d, global FO printing: %d, candidates list: %d, N: %d predictor: %d",
    		par.filename,
    		par.cache,
			par.maxIterations,
			par.debug,
			par.globalFOcalculation,
			par.K,
			par.number,
			par.predictor
    );debugMessage(debugMsg, par);
    // This return code is tested by the caller
    // Any value different than 0 will fire an exception
    return 0;

}
