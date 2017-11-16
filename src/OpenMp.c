/*
 * OpenMp.c
 *
 *  Created on: Nov 5, 2017
 *      Author: work
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>
#include <sys/time.h>


#include "common.h"

void calculateOpenMPBounds(sApplication * pointer, int n_threads, sConfiguration * configuration, MYSQL *conn, struct optJrParameters par)
{
	printf("\n\n*************** Calculate bounds (OpenMP) for each application *************** \n\n");

	sApplication* t_pointer[n_threads];
	MYSQL *conn2[n_threads];
	for (int i =0; i< n_threads;++i)
	{
		t_pointer[i]=pointer;
		conn2[i]=DBopen(
								getConfigurationValue(configuration, "DB_IP"),
						getConfigurationValue(configuration, "DB_user"),
						getConfigurationValue(configuration, "DB_pass"),
						getConfigurationValue(configuration, "DB_dbName")
						);
	}
 	//call findbound in parallel;
	#pragma omp parallel num_threads(n_threads)
	{
		int ID=omp_get_thread_num();
		int j=0;
		char debugMsg[DEBUG_MSG];
		char statement[256];

		while(t_pointer[ID]!=NULL )// assign each app to a thread
		{
			int pos=j%n_threads;
			//printf("\n\n<debug message>: pos= %d, j=%d, ID=%d\n\n\n", pos,j,ID);

			if(pos==ID)
			{
				printf("\n<debug message>: findBound of app number %d called from thread %d\n",j,ID);

				/* Retrieve the value calculated by OPT_IC from DB */
				sprintf(debugMsg,"\n calculateBounds %s %s\n", t_pointer[ID]->session_app_id, t_pointer[ID]->app_id);debugMessage(debugMsg, par);
				//Retrieve nCores from the DB
				sprintf(statement,
				                        "select num_cores_opt, num_vm_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%lf;"
								, getConfigurationValue(configuration, "DB_dbName"), t_pointer[ID]->app_id, t_pointer[ID]->datasetSize, t_pointer[ID]->Deadline_d);

				MYSQL_ROW row = executeSQL(conn2[ID], statement, par);
				if (row == NULL)
				{
					printf("Fatal error: no matches found on OPTIMIZER_CONFIGURATION_TABLE.\nCHeck that OPT_IC was run over the specific application.\n");
					exit(-1);
				}

				t_pointer[ID]->nCores_DB_d = atoi(row[0]);
				t_pointer[ID]->vm = atoi(row[1]);

				Bound(configuration, conn2[ID], t_pointer[ID], par);
				sprintf(debugMsg,"A bound for %s has been calculated", t_pointer[ID]->session_app_id);
				debugMessage(debugMsg, par);
			}
			t_pointer[ID]=t_pointer[ID]->next;
			++j;
		}
	}
	printf("\n\n*************** End calculate bounds ***************** \n\n");

}



void invokePredictorOpenMP(sCandidates * pointer,  struct optJrParameters par, sConfiguration * configuration )
{
	int n_threads = par.numberOfThreads;
	char debugMsg[256];
	sCandidates* t_pointer[n_threads];
	MYSQL *conn2[n_threads];
	int backup[n_threads];

	if (pointer == NULL) return;

	debugBanner("invokePredictorInAdvance", par);
	for (int i = 0; i< n_threads;++i)
	{
			conn2[i]=DBopen(
								getConfigurationValue(configuration, "DB_IP"),
								getConfigurationValue(configuration, "DB_user"),
								getConfigurationValue(configuration, "DB_pass"),
								getConfigurationValue(configuration, "DB_dbName")
								);
			t_pointer[i]=pointer;
	}


 	//call invokePredictorInAdvance in parallel
	#pragma omp parallel num_threads(n_threads)
	{
		int ID=omp_get_thread_num();
		int j=0;


		while(t_pointer[ID]!=NULL )// assign each app to a thread
		{
			int pos=j % n_threads;

			if(pos==ID)
			{
				//sprintf(debugMsg,"invokePredictorOpenMP: pos= %d, j=%d, ID=%d n_threads=%d", pos,j,ID, n_threads);debugMessage(debugMsg, par);
				//sprintf(debugMsg,"invokePredictorOpenMP of app number %d called from thread %d",j,ID);debugMessage(debugMsg, par);
				t_pointer[ID]->real_i = ObjFunctionComponent(configuration, conn2[ID], t_pointer[ID]->app_i, par);
				t_pointer[ID]->nodes_i = t_pointer[ID]->app_i->currentCores_d;
				t_pointer[ID]->real_j = ObjFunctionComponent(configuration, conn2[ID], t_pointer[ID]->app_j, par);
											t_pointer[ID]->nodes_j = t_pointer[ID]->app_j->currentCores_d;

				//sprintf(debugMsg,"invokePredictorOpenMP: app_i %s app_j %s real_i %lf real_j %lf\n", t_pointer[ID]->app_i->app_id, t_pointer[ID]->app_j->app_id, t_pointer[ID]->real_i, t_pointer[ID]->real_j);debugMessage(debugMsg, par);
			}
			t_pointer[ID]=t_pointer[ID]->next;
			++j;
		}


	}


}
