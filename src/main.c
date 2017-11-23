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
    par.numberOfThreads = atoi(getConfigurationValue(configuration, "THREADS_NUMBER"));

    /* Connect to the db */
    MYSQL *conn = DBopen(
            			getConfigurationValue(configuration, "OptDB_IP"),
    					getConfigurationValue(configuration, "OptDB_user"),
    					getConfigurationValue(configuration, "OptDB_pass"),
    					getConfigurationValue(configuration, "OptDB_dbName")
    					);
    if (conn == NULL) DBerror(conn, "open_db: Opening the database");

    // Calculate the time taken
    gettimeofday(&tv_initial_main, NULL);

    /* Load applications details from csv file */
    sApplication *first = parseCsv(configuration, par);

    /* Calculate the indices */
    gettimeofday(&tv_initial_nu, NULL);
    calculate_Nu(configuration, conn, first,  par);
    gettimeofday(&tv_final_nu, NULL);

    /* Calculate the bounds */
    gettimeofday(&tv_initial_bounds, NULL);
    if (par.numberOfThreads == 0) calculateBounds(first, configuration, conn, par);
    	else calculateOpenMPBounds(first, par.numberOfThreads, configuration, conn, par);
    printApplications(first, par);
     gettimeofday(&tv_final_bounds, NULL);


     /* Fix initial solution */
    gettimeofday(&tv_initial_fix, NULL);
    sApplicationPointers *firstPointer = fixInitialSolution(first, par);
    gettimeofday(&tv_final_fix, NULL);

    /* Calculate baseFO for each application */
    gettimeofday(&tv_initial_init, NULL);
    initialize(configuration, conn, first, par);
    gettimeofday(&tv_final_init, NULL);

    /* Invoke localSearch */
    gettimeofday(&tv_initial_locals, NULL);
    localSearch(configuration, conn, first, par);
    gettimeofday(&tv_final_locals, NULL);

    debugInformational("Final solution\n", par);
    writeResults(conn, getConfigurationValue(configuration,"OptDB_dbName"),first, par);

    /* De-allocate resources and close connection */
    freeParameters(first);
    freeApplications(firstPointer);

    DBclose(conn);

    gettimeofday(&tv_final_main, NULL);

    //printOutput(first);
    sprintf(debugMsg, "FixInitial step elapsed time: %lf\n", elapsedTime(tv_initial_fix, tv_final_fix));debugMessage(debugMsg, par);
    sprintf(debugMsg, "Findbounds  elapsed time: %lf\n", elapsedTime(tv_initial_bounds, tv_final_bounds));debugMessage(debugMsg, par);
    sprintf(debugMsg, "Initialization elapsed time %lf\n", elapsedTime(tv_initial_nu, tv_final_nu));debugMessage(debugMsg, par);
    sprintf(debugMsg, "LocalSearch step elapsed time: %lf\n", elapsedTime(tv_initial_locals, tv_final_locals));debugMessage(debugMsg, par);
    sprintf(debugMsg, "Overall elapsed time: %lf\n", elapsedTime(tv_initial_main, tv_final_main));debugMessage(debugMsg, par);

    // This return code is tested by the caller
    // Any value different than 0 will fire an exception
    return 0;

}
