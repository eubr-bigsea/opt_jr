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

#include "predictor.h"






/*
 * Name: invokePredictor
 * Input parameters: sConfiguration * configuration, MYSQL *conn, int nNodes, int currentCores, char * memory, int datasize,  char *sessionId, char *appId, char *stage, struct optJrParameters par, int flagDagsim
 * Output parameters: char * (dagSim output)
 * Description: It invokes a predictor (dagSim/Lundstrom). First it checks if an estimate of the execution time is already stored in the DB; if not, it invokes the actual predictor
 * 				and stores the result on DB cache table.
 */
char* invokePredictor(sConfiguration * configuration, MYSQL *conn, int currentCores, char *sessionId, char *appId, char *stage,
		struct optJrParameters par, int flagDagsim, char * luafilename, char *results, int mode)
{
	char parameters[1024];
	char cmd[1024];

	char lua[1024];

	char statementSearch[1024], statement[1024];
	char debugMsg[DEBUG_MSG];
	char dbName[64];


	if (currentCores == 0)
	{
		printf("Fatal Error: invokePredictor: currentCores cannot be equal zero\n");
		exit(-1);
	}
	/* Create unique extension */
	struct timeval tv;
	gettimeofday(&tv,NULL);
	double extension = (double)tv.tv_usec;
	char *output1 = (char *)malloc(64);
	if (output1 == NULL)
	{
		printf("Malloc failure: invokePredictor: output1\n");
		exit(-1);
	}
	//debugBanner("invokePredictor", par);

	/* Create unique dagSim output filename */
	double systemTime;
	double stageTime;
	char predictorOutputPath[256];
	sprintf(predictorOutputPath, "/tmp/outputDagsim_%lf.txt", extension);

	/* Extract nodes, cores, dataset and memory from the subfolder name */

	char _nodes[8], _cores[8], _memory[8], _dataset[8], temp[8];

	char *dir = (char *)malloc(1024);

	strcpy(dir, getConfigurationValue(configuration, results));
	strcpy(dir, readFolder(dir, 1));

	strcpy(_nodes, extractItem(dir, "", "_"));

	strcpy(_cores, extractItem(dir, "_", "_"));

	sprintf(temp, "%s_%s_", _nodes, _cores);
	strcpy(_memory, extractItem(dir, temp, "_"));

	sprintf(temp, "%s_%s_%s_", _nodes, _cores, _memory);
	strcpy(_dataset, extractItem(dir, temp, NULL));


	//int TOTAL_NODES = atoi(_nodes) * currentCores;
	int TOTAL_NODES = currentCores;
	//printf("_nodes %s _cores %s _memory %s _dataset %s\ TOTAL_NODES %d\n", _nodes, _cores, _memory, _dataset, TOTAL_NODES);exit(12);


	MYSQL_ROW row = NULL;
	/* Determine the predictor to invoke */
	switch(par.predictor)
	{
			case LUNDSTROM:
				/* Adjust first the config file */
				//updateLundstromConfig(configuration, results, mode);
				sprintf(parameters, "-n %s -c %s -r %s -d %s -q %s -p spark -k %d > %s", _nodes, _cores, _memory, _dataset, appId, TOTAL_NODES, predictorOutputPath);
				//if (strcmp(results, "RESULTS_HOME") == 0)
						sprintf(cmd, "cd %s;python run.py %s", getConfigurationValue(configuration, "LUNDSTROM_HOME"), parameters);
				//				else sprintf(cmd, "cd %s;python run.py %s", getConfigurationValue(configuration, "ALTERNATIVE_LUNDSTROM_HOME"), parameters);
				printf("%s\n", cmd);
				/* Execute predictor */
				_run(cmd, par);
				/* Parse the output */
				strcpy(output1,
						extractItem(readFile(predictorOutputPath),
						"predicted\": \"", "\""));
				break;
			case DAGSIM:
				if (par.cache == YES)
				{
					sprintf(debugMsg, "invokePredictor: session %s app %s checking cache on %d cores\n", sessionId, appId, TOTAL_NODES);debugMessage(debugMsg, par);
					/* Check first if the estimate time has been already calculated */
					strcpy(dbName, getConfigurationValue(configuration,"DB_dbName"));
					sprintf(statementSearch, "select %s.PREDICTOR_CACHE_TABLE.val \nfrom %s.PREDICTOR_CACHE_TABLE,"
									"%s.APPLICATION_PROFILE_TABLE \nwhere %s.PREDICTOR_CACHE_TABLE.is_residual = %d and "
											"%s.PREDICTOR_CACHE_TABLE.dataset_size = "
									"%s.APPLICATION_PROFILE_TABLE.dataset_size and\n 	"
									"%s.PREDICTOR_CACHE_TABLE.application_id=\'%s\' and %s.PREDICTOR_CACHE_TABLE.dataset_size=%s  "
									"\nand %s.PREDICTOR_CACHE_TABLE.num_cores = %d and %s.PREDICTOR_CACHE_TABLE.stage = \'%s\';\n",
									dbName,
									dbName,
									dbName,
									dbName,
									flagDagsim,
									dbName,
									dbName,
									dbName,appId, dbName,_dataset, dbName,TOTAL_NODES, dbName, stage);

				 row = executeSQL(conn, statementSearch, par);
				//sprintf(debugMsg, "statement %s\n", statementSearch);debugMessage(debugMsg, par);
				}

				if (row == NULL || par.cache == NO)
				{
					strcpy(lua, luafilename);
					char pattern[64], newpath[256];
					sprintf(pattern, "Nodes = %d",TOTAL_NODES);

					/* Create unique LUA filename */

					sprintf(newpath, "/tmp/temp_%lf.lua", extension);
					writeFile(newpath, replace(readFile(lua), pattern));

					/* Using /tmp/temp.lua instead of lua variable, so the original file is never overwritten */
					sprintf(cmd, "cd %s;./dagsim.sh %s -s > %s", getConfigurationValue(configuration, "DAGSIM_HOME"), newpath, predictorOutputPath);
					sprintf(debugMsg, "Executing predictor: %s", cmd);debugMessage(debugMsg, par);
					_run(cmd, par);

					/* Parse the time value (either total time or residula time) */
					switch(flagDagsim)
					{
						case RESIDUAL_EXECUTION_TIME:
							systemTime = atof(extractWord(extractRowN(readFile(predictorOutputPath),3), 3));
							stageTime = atof(extractWord(extractRowN(extractRowMatchingPattern(readFile(predictorOutputPath), stage),1), 4));
							sprintf(output1, "%lf", (systemTime - stageTime));
							sprintf(debugMsg, "Residual time: %s", output1);debugMessage(debugMsg, par);
							break;

						case WHOLE_EXECUTION_TIME:
							strcpy(output1, extractWord(extractRowN(readFile(predictorOutputPath),1),3));
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


					if (par.cache == YES)
					{
						/* Update the db cash table with a new value */
						/* Check again that no other MPI opt_jr instance has updated the DB */
						row = executeSQL(conn, statementSearch, par);
						if (row == NULL)
						{
							sprintf(statement,"insert %s.PREDICTOR_CACHE_TABLE values('%s', %s, %d, '%s', %d, %lf);",
							getConfigurationValue(configuration, "DB_dbName"),
							appId,
							_dataset,
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

