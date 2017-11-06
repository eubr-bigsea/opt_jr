/*
 * io.c
 *
 *  Created on: 10 Mar 2017
 *      Author: Enrico
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

    double N;
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



    /*
     * Read total cores available
     */
    N = par.number;
    int MAX_PROMISING_CONFIGURATIONS = par.K;

    /* Load applications details from csv file */
    sApplication *first = parseCsv(configuration, par);

    /* Calculate the bounds */
        gettimeofday(&tv_initial_bounds, NULL);
        calculateBounds(first, configuration, conn, par);
        //calculateMPIBounds(first, par.numberOfThreads, configuration, conn, par);
        gettimeofday(&tv_final_bounds, NULL);
        printApplications(first, par);

    gettimeofday(&tv_initial_nu, NULL);
     /* Calculate the indices */
     calculate_Nu(configuration, conn, first,  par);
     gettimeofday(&tv_final_nu, NULL);

    gettimeofday(&tv_initial_fix, NULL);

    sApplicationPointers *firstPointer = fixInitialSolution(first, par);
    gettimeofday(&tv_final_fix, NULL);

    gettimeofday(&tv_initial_init, NULL);
       /* Calculate baseFO for each application */
       initialize(configuration, conn, first, par);
       gettimeofday(&tv_final_init, NULL);

    /* Invoke localSearch */
    gettimeofday(&tv_initial_locals, NULL);
    localSearch(configuration, conn, first, N, MAX_PROMISING_CONFIGURATIONS, par);
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
