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
#include <mpi.h>
#include <sys/time.h>

#include "predictor.h"

/*
 * Name: invokePredictor
 * Input parameters: sConfiguration * configuration, MYSQL *conn, int nNodes, int currentCores, char * memory, int datasize,  char *sessionId, char *appId, char *stage, struct optJrParameters par, int flagDagsim
 * Output parameters: char * (dagSim output)
 * Description: It invokes a predictor (dagSim/Lundstrom). First it checks if an estimate of the execution time is already stored in the DB; if not, it invokes the actual predictor
 * 				and stores the result on DB cache table.
 */
char* invokePredictor(sConfiguration * configuration, MYSQL *conn, int nNodes, int currentCores, char * memory, int datasize,  char *sessionId, char *appId, char *stage, struct optJrParameters par, int flagDagsim)
{
	char parameters[1024];
	char cmd[1024];
	char path[1024];
	char lua[1024];
	char subfolder[1024];
	char statementSearch[1024], statement[1024];
	char debugMsg[DEBUG_MSG];
	char dbName[64];
	char dir[1024];
	int TOTAL_NODES = nNodes * currentCores;
	char *output1 = (char *)malloc(64);
	if (output1 == NULL)
	{
		printf("Malloc failure: invokePredictor: output1\n");
		exit(-1);
	}
	//debugBanner("invokePredictor", par);

	if (nNodes == 0)
	{
		printf("Fatal error: invokePredictor; nNodes cannot be 0\n");
		exit(-1);
	}
	/* Consider always the same log folder and lua file (replacing "on the fly" the number of nodes)
	 This is possible because the variance between the log folders is small*/
	strcpy(path, getConfigurationValue(configuration, "RESULTS_HOME"));
	strcpy(dir, readFolder(path));
	/* Determine the predictor to invoke */
	switch(par.predictor)
	{
			case LUNDSTROM:
				sprintf(parameters, "%d %d %s %d %s", nNodes, currentCores, memory, datasize, appId);
				sprintf(cmd, "cd %s;python run.py %s", getConfigurationValue(configuration, "LUNDSTROM_HOME"), parameters);
				break;
			case DAGSIM:
				/* Check first if the estimate time has been already calculated */
				strcpy(dbName, getConfigurationValue(configuration,"DB_dbName"));
				sprintf(statementSearch, "select %s.PREDICTOR_CACHE_TABLE.val \nfrom %s.PREDICTOR_CACHE_TABLE,"
									"%s.APPLICATION_PROFILE_TABLE \nwhere %s.PREDICTOR_CACHE_TABLE.is_residual = %d and "
											"%s.PREDICTOR_CACHE_TABLE.dataset_size = "
									"%s.APPLICATION_PROFILE_TABLE.dataset_size and\n 	"
									"%s.PREDICTOR_CACHE_TABLE.application_id=\'%s\' and %s.PREDICTOR_CACHE_TABLE.dataset_size=%d  "
									"\nand %s.PREDICTOR_CACHE_TABLE.num_cores = %d and %s.PREDICTOR_CACHE_TABLE.stage = \'%s\';\n",
									dbName,
									dbName,
									dbName,
									dbName,
									flagDagsim,
									dbName,
									dbName,
									dbName,appId, dbName,datasize, dbName,TOTAL_NODES, dbName, stage);

				MYSQL_ROW row = executeSQL(conn, statementSearch, par);
				sprintf(debugMsg, "statement %s\n", statementSearch);debugMessage(debugMsg, par);

				if (row == NULL || par.cache == 0)
				{
					sprintf(debugMsg, "Last SQL statement returned 0 rows or cache is disabled. Invoking predictor...");debugMessage(debugMsg, par);
					sprintf(path, "%s/%s/%s/logs", getConfigurationValue(configuration, "RESULTS_HOME"),readFolder(path), appId);
					strcpy(subfolder, readFolder(path));
					sprintf(cmd, "%s/%s/", path, subfolder);
					sprintf(cmd, "%s*.lua", cmd);
					strcpy(lua, ls(cmd, par));

					char pattern[64], newpath[256];
					sprintf(pattern, "Nodes = %d",(nNodes*currentCores));
					/* Create unique extension (it can be improved) */
					struct timeval tv;
					gettimeofday(&tv,NULL);

					/* Create unique LUA filename */
					double extension = (double)tv.tv_usec;
					sprintf(newpath, "/tmp/temp_%lf.lua", extension);
					writeFile(newpath, replace(readFile(lua), pattern));

					/* Create unique dagSim output filename */
					double systemTime;
					double stageTime;
					char dagSimOutputPath[256];
					sprintf(dagSimOutputPath, "/tmp/outputDagsim_%lf.txt", extension);

					/* Using /tmp/temp.lua instead of lua variable, so the original file is never overwritten */
					sprintf(cmd, "cd %s;./dagsim.sh %s -s > %s", getConfigurationValue(configuration, "DAGSIM_HOME"), newpath, dagSimOutputPath);
					sprintf(debugMsg, "Executing predictor: %s", cmd);debugMessage(debugMsg, par);
					_run(cmd, par);

					/* Parse the time value (either total time or residula time) */
					switch(flagDagsim)
					{
						case RESIDUAL_EXECUTION_TIME:
							systemTime = atof(extractWord(extractRowN(readFile(dagSimOutputPath),3), 3));
							stageTime = atof(extractWord(extractRowN(extractRowMatchingPattern(readFile(dagSimOutputPath), stage),1), 4));
							sprintf(output1, "%lf", (systemTime - stageTime));
							sprintf(debugMsg, "Residual time: %s", output1);debugMessage(debugMsg, par);
							break;

						case WHOLE_EXECUTION_TIME:
							strcpy(output1, extractWord(extractRowN(readFile(dagSimOutputPath),1),3));
							break;

						default:
							printf("FATAL ERROR: invokePredictor: unknown case %d\n", flagDagsim);
							exit(-1);
							break;
					}
					/* Check for anomalies */
					if (doubleCompare(atof(output1), 0) == 0)
					{
						printf("Fatal Error: invokePredictor: dagSim output was zero (%s)\n", newpath);
						exit(-1);
					}



					/* Update the db cash table with a new value */
					/* Check again that no other MPI opt_jr instance has updated the DB */
					row = executeSQL(conn, statementSearch, par);
					if (row == NULL)
					{
						sprintf(statement,"insert %s.PREDICTOR_CACHE_TABLE values('%s', %d, %d, '%s', %d, %lf);",
							getConfigurationValue(configuration, "DB_dbName"),
							appId,
							datasize,
							TOTAL_NODES,
							stage,
							flagDagsim,
							atof(output1)
							);
						sprintf(debugMsg, "Executing %s", statement);debugMessage(debugMsg, par);
						if (mysql_query(conn, statement))
						{
							char error[512];
							sprintf(error, " %s", statement);
							DBerror(conn, error);
						}

					}

				} /* If no value was stored in the cache */
				else
				{
					double out = atof(row[0]);
					sprintf(debugMsg,"Dagsim output retrieved from DB cash: %s %lf", appId, out);debugMessage(debugMsg, par);
					sprintf(output1, "%lf", out);
				}
				break;
		} /* Switch predictor */

	/* Return the predictor output */
	return output1;
}

