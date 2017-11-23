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
#include <limits.h>
#include <float.h>
#include <sys/time.h>


#include "common.h"


void printOPT_JRPars(struct optJrParameters par )
{
	printf("N: %d\n", par.number);
	printf("Filename: %s\n", par.filename);
	printf("Candidates List Limit: %d\n", par.K);
	printf("DEBUG option: %d\n", par.debug);
	printf("Simulator: %d\n", par.predictor);
	printf("Global FO calculation: %d\n", par.globalFOcalculation);
}
/*
 * 		Name:					addApplication
 * 		Input parameters:		int nApp, sApplication ** first, sApplication ** current,  char * app_id, double w, double chi_0, double chi_C, double chi_c_1, double m, double M, double V, double v, int D, double csi,
		double csi_1, char * StageId, int datasetSize
 * 		Output parameters:		Updated pointers to the first and current element of the list
 * 		Description:			This function adds all the information regarding an application into a weight-sorted list
 *
 */
void addApplication(sApplication ** first,   sApplication ** current, char *session_app_id, char * app_id, double w, double chi_0, double chi_C, double m, double M, double V, double v, double Deadline_d, double csi,
		char * StageId, int datasetSize)
{
	  sApplication *new = (sApplication*) malloc(sizeof(sApplication));
	  if (new == NULL)
	  {
		  printf("Fatal Error:addParameters:  malloc failure\n");
		  exit(-1);
	  }

	  new->w = w;
	  new->app_id = (char *)malloc(1024);
	  if (new->app_id == NULL)
	  {
	  	  	    printf("Fatal Error:addParameters: malloc failure\n");
	  	  	    exit(-1);
	  }
	  strcpy(new->app_id, app_id);

	  new->session_app_id = (char *)malloc(1024);
	  if (new->session_app_id == NULL)
	  {
	  	  	printf("Fatal Error:addParameters: malloc failure\n");
	  	  	exit(-1);
	  }
	  strcpy(new->session_app_id, session_app_id);

	  new->chi_0 = chi_0;
	  new->chi_C = chi_C;
	  new->m = m;
	  new->M = M;
	  new->V = V;
	  new->v = v;
	  new->Deadline_d = Deadline_d;
	  new->csi = csi;
	  new->boundIterations = 0;
	  new->currentCores_d = 0;
	  new->nCores_DB_d = 0;

	  new->stage = (char *)malloc(1024);
	  if (new->stage == NULL)
	  {
		  printf("addParameters: malloc failure (stage)\n");
		  exit(-1);
	  }
	  strcpy(new->stage, StageId);
	  new->datasetSize = datasetSize;

	  /* Initialize the parameter that will be calculated later */
	  new->R_d = 0;
	  new->baseFO = -1;
	  new->bound = 0;

	  new->next = NULL;

	  /*
	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
	   */

	  if (*first == NULL) *first = new;
	 	 	 	  else
	 	 	 		  if (doubleCompare((*first)->w, w) == 1)
	 	 	 		  {
	 	 	 			  new->next = *first;
	 	 	 			  *first = new;
	 	 	 		  }
	 	 	 		  	 else
	 	 	 			 {
	 	 	 		  		sApplication * previous = *first;
	 	 	 		  	sApplication * current = (*first)->next;

	 	 	 				 while (current != NULL && current->w < w)
	 	 	 				 {
	 	 	 					 previous = current;
	 	 	 					 current = current->next;
	 	 	 				 }

	 	 	 				 previous->next = new;
	 	 	 				 new->next = current;
	 	 	 			 }
}

/*
 * 		Name:					writeResults
 * 		Input parameters:		sApplication *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			This function prints the results of the localSearch application (number of cores and VM in a DB table.
 * 								If a result for a (session_id, application_id) already exists, then it is replaced.
 *
 */


void writeResults(MYSQL *conn, char * dbName, sApplication *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	char sqlStatement[512];

	debugBanner( "writeResults", par);
	if (pointer == NULL)
	{
		printf("FATAL ERROR: writeResults: pointer cannot be null\n");
		exit(-1);
	}
	while (pointer!=NULL)
	{
		printf("Session ID %s Application Id %s cores %d VMs %d\n", pointer->session_app_id, pointer->app_id, pointer->currentCores_d, pointer->vm);debugMessage(debugMsg, par);

		/* Check if the result of the computation for that session, application has been already computed and stored previously */
		sprintf(sqlStatement, "select opt_id, app_id from %s.OPT_SESSIONS_RESULTS_TABLE where opt_id='%s' and app_id='%s'",
				dbName, par.filename,
				pointer->session_app_id);
		MYSQL_ROW row = executeSQL(conn, sqlStatement, par);

		if (row == NULL)
		{
			sprintf(sqlStatement, "insert %s.OPT_SESSIONS_RESULTS_TABLE values('%s', '%s',%d, %d)",
								dbName,
								par.filename,
								pointer->session_app_id,
								pointer->currentCores_d,
								pointer->vm
						);
			if (mysql_query(conn, sqlStatement))
			{
				char error[512];
				sprintf(error, " %s", sqlStatement);
				DBerror(conn, error);
			}
		}
		else /* Perform an update */
		{
			sprintf(sqlStatement, "update %s.OPT_SESSIONS_RESULTS_TABLE set opt_id = '%s', app_id = '%s',num_cores = %d, num_vm = %d where opt_id='%s' and app_id='%s'",
											dbName,
											par.filename,
											pointer->session_app_id,
											pointer->currentCores_d,
											pointer->vm,
											par.filename,
											pointer->session_app_id
									);
			if (mysql_query(conn, sqlStatement))
						{

							char error[512];
							sprintf(error, " %s", sqlStatement);
							DBerror(conn, error);
						}
		}
		pointer = pointer->next;
	}

}

/*
 * 		Name:					readApplication
 * 		Input parameters:		sApplication *pointer, struct optJrParameters par
 * 		Description:			It prints the details of all the applications
*/
void printApplications(sApplication *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "Applications list content:");debugInformational(debugMsg, par);

	while (pointer!=NULL)
	{
		printApplication(pointer, par);
		pointer = pointer->next;
	}

}


void printApplication(sApplication *pointer, struct optJrParameters par)
{

	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "session_app_id %s app_id %s  weight %lf nu %lf iterations to find the bound %d currentcores = %d nCores from DB = %d",
			pointer->session_app_id, pointer->app_id, pointer->w, pointer->nu_d, pointer->boundIterations, pointer->currentCores_d, (int)pointer->nCores_DB_d);debugMessage(debugMsg, par);

}

/*
 * CURRENTLY NOT USED -> WILL BE REMOVED AFTER TESTS HAVE BEEN CONPLETED
 */
void commitAssignment(sApplication *pointer, char *session_appId,  double DELTA, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	while (pointer != NULL)
		if (strcmp(pointer->session_app_id, session_appId) == 0) break;
		else pointer = pointer->next;

	if (pointer == NULL)
	{
		printf("Application %s not found in the list\n", session_appId);
		exit(-1);
	}

	/* Check that currentCores_d is positive: ignore if otherwise */
	if (pointer->currentCores_d + DELTA*pointer->V <= 0)
	{
		printf("Negative or zero value for currentCores was not committed\n");
		return;
	}


	pointer->currentCores_d = pointer->currentCores_d + DELTA*pointer->V;
	sprintf(debugMsg, "Committed %s currentCores = %d\n", pointer->session_app_id, (int)pointer->currentCores_d);debugMessage(debugMsg, par);


}

/*
 * 		Name:					freeApplications
 * 		Input parameters:		sApplication *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			It releases the allocated memory for the list
 *
 */
void freeApplications(sApplicationPointers * pointer)
{
	sApplicationPointers * tmp;


		while (pointer != NULL)
		    {
		       tmp = pointer;
		       pointer = pointer->next;
		       free(tmp);
		    }
}

/*
 * 		Name:					freeParameters
 * 		Input parameters:		sApplication *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			It releases the allocated memory for the list
 *
 */
void freeParameters(sApplication * pointer)
{
	sApplication * tmp;

	while (pointer != NULL)
			    {
			       tmp = pointer;
			       pointer = pointer->next;
			       free(tmp);
			    }
}


/*
 * 		Name:					freeCandidateList
 * 		Input parameters:		sCandidates *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			It releases the allocated memory for the list
 *
 */
void freeCandidates(sCandidates * pointer)
{
	sCandidates * tmp;
	while (pointer != NULL)
			    {
			       tmp = pointer;
			       pointer = pointer->next;
			       free(tmp);
			    }

}

void freeStatistics(sStatistics * pointer)
{
	sStatistics * tmp;

	while (pointer != NULL)
			    {
			       tmp = pointer;
			       pointer = pointer->next;
			       free(tmp);
			    }
}

/*
 * 		Name:					addCandidates
 * 		Input parameters:		sCandidates ** first, sCandidates ** current,  char * app_id1, char * app_id2, int contr1, int contr2, double delta
 * 		Output parameters:		Updated pointers to the first and current element of the list
 * 		Description:			This function adds all the information regarding the localSearch deltafo calculation. The list is sorted by deltafo value.
 *
 */
void addCandidate(sCandidates ** first, sCandidates ** current,  sApplication * app_i, sApplication * app_j, int contr1, int contr2, double delta, double delta_i, double delta_j)
{
	  sCandidates *new = (sCandidates*) malloc(sizeof(sCandidates));
	  if (new == NULL)
	  {
		  printf("addCandidateParameters: Fatal Error: malloc failure\n");
		  exit(-1);
	  }

	  new->app_i = app_i;
	  new->app_j = app_j;
	  new->newCoreAssignment_i = contr1;
	  new->newCoreAssignment_j = contr2;
	  new->deltaFO = delta;
	  new->delta_i = delta_i;
	  new->delta_j = delta_j;
	  new->next = NULL;

/*
	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;*/

	  if (*first == NULL) *first = new;
	 	 	 	  else
	 	 	 		  if (doubleCompare((*first)->deltaFO, delta) == 1)
	 	 	 		  {
	 	 	 			  new->next = *first;
	 	 	 			  *first = new;
	 	 	 		  }
	 	 	 		  	 else
	 	 	 			 {
	 	 	 		  		sCandidates * previous = *first;
	 	 	 		  		sCandidates * current = (*first)->next;

	 	 	 				 while (current != NULL && doubleCompare(current->deltaFO, delta) == -1)
	 	 	 				 {
	 	 	 					 previous = current;
	 	 	 					 current = current->next;
	 	 	 				 }

	 	 	 				 previous->next = new;
	 	 	 				 new->next = current;
	 	 	 			 }
}


/*
 * 		Name:					addStatistics
 * 		Input parameters:		sStatistics ** first, sStatistics ** current, int iteration, int how_many
 * 		Output parameters:		current pointer to the list
 * 		Description:			This function adds a new statistics (iteration_id, number of candidates found by hyperbolic approximation
 *
 */
void addStatistics(sStatistics ** first, sStatistics ** current, int iteration, int how_many, double total)
{


	  sStatistics *new = (sStatistics*) malloc(sizeof(sStatistics));
	  if (new == NULL)
	  {
		  printf("addCandidateParameters: Fatal Error: malloc failure\n");
		  exit(-1);
	  }

	  new->iteration = iteration;
	  new->size = how_many;
	  new->FO_Total = total;
	  new->next = NULL;


	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;

}


/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */
void addConfiguration(sConfiguration ** first, sConfiguration ** current, char * variable, char * value)
{


	  sConfiguration *new = (sConfiguration*) malloc(sizeof(sConfiguration));
	  if (new == NULL)
	  {
		  printf("addConfiguration: Fatal Error: malloc failure\n");
		  exit(-1);
	  }

	  new->variable = (char *)malloc(256);
	  if (new->variable == NULL)
	  {
		  printf("Malloc failure: addConfiguration\n");
		  exit(-1);
	  }
	  new->value = (char *)malloc(256);
	  if (new->value == NULL)
	  {
	  		printf("Malloc failure: addConfiguration\n");
	  		exit(-1);
	  }

	  strcpy(new->variable, variable);
	  strcpy(new->value, value);
	  new->next = NULL;


	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
}

/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */
sConfiguration * readConfigurationFile()
{
	FILE * fp;
	    char * line = NULL;
	    size_t len = 0;
	    ssize_t read;
	    char var[256], val[256],* configurationFile;

	    sConfiguration *first = NULL;
	    sConfiguration *current = NULL;

	    configurationFile = getenv("HOME");
	    configurationFile = strcat(configurationFile, "/wsi_config.xml");

	    if (configurationFile == NULL)
	    {
	    	printf("Fatal error: parseConfigurationFile%s/wsi_config.xml configuration file not found.\n", getenv("HOME"));
	    	exit(-1);
	    }

	    fp = fopen(configurationFile , "r");
	    if (fp == NULL)
	    {
	    	printf("Fatal Error: parseConfigurationFile: configuration file not found in home directory: (%s)\n", configurationFile);
	    	exit(-1);
	    }



	    while ((read = getline(&line, &len, fp)) != -1)
	    {
	       if (line != NULL)
	    	if (strstr(line, "entry key=") != NULL)
	    	{
	    		strcpy(var, extractItem(line, "\"", "\""));
	    		strcpy(val, extractItem(line, ">", "<"));

	    		addConfiguration(&first, &current, var, val);
	    	}

	    }

	    return first;
}


/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */
char *getConfigurationValue(sConfiguration *pointer, char * variable)
{
	int found = 0;

	while (!found && pointer != NULL)
	{
		if (strcmp(pointer->variable, variable) == 0) found = 1;
		else pointer = pointer->next;
	}
	if (!found)
	{
		printf("variable %s not found!\n", variable);
		exit(-1);

	}
	else return pointer->value;
}

/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */
void readStatistics(sStatistics *pointer, struct optJrParameters par)
{


	debugBanner("Statistics list content:", par);

	if (par.numberOfThreads > 0) printf("(OpenMP: yes) Iteration   List Size  Total_FO\n");
	else printf("(OpenMP: no) Iteration   List Size  Total_FO\n");
	while (pointer!=NULL)
	{
		printf("%d %d %lf\n", pointer->iteration, pointer->size, pointer->FO_Total);
		pointer = pointer->next;
	}

}

/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */
void addApplicationPointer(sApplicationPointers ** first,   sApplication *application)
{


	  sApplicationPointers *new = (sApplicationPointers*) malloc(sizeof(sApplicationPointers));
	  if (new == NULL)
	  {
		  printf("addListPointers: Fatal Error: malloc failure\n");
		  exit(-1);
	  }

	  new->app= application;
	  new->next = NULL;

	  /*
	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
	  */


	  if (*first == NULL) *first = new;
	 	 	 	 	  else
	 	 	 	 		  if (doubleCompare((*first)->app->w, application->w) == 1)
	 	 	 	 		  {
	 	 	 	 			  new->next = *first;
	 	 	 	 			  *first = new;
	 	 	 	 		  }
	 	 	 	 		  	 else
	 	 	 	 			 {
	 	 	 	 		  		sApplicationPointers * previous = *first;
	 	 	 	 		  		sApplicationPointers * current = (*first)->next;

	 	 	 	 				 while (current != NULL && doubleCompare(current->app->w, application->w) == -1)
	 	 	 	 				 {
	 	 	 	 					 previous = current;
	 	 	 	 					 current = current->next;
	 	 	 	 				 }

	 	 	 	 				 previous->next = new;
	 	 	 	 				 new->next = current;
	 	 	 	 			 }

}

/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */
void readApplicationPointers(sApplicationPointers *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "\n\nListPointers list content:\n");debugMessage(debugMsg, par);


	while (pointer!=NULL)
	{
		printApplication(pointer->app, par);
		pointer = pointer->next;
	}
	sprintf(debugMsg, "\n");debugMessage(debugMsg, par);
}



/*
 * 		Name:					readCandidateList
 * 		Input parameters:		sCandidates *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			This function prints the information about all the applications in the list. It is used for debug only.
 *
 */


void readCandidates(sCandidates *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "\n\nCandidate application list content:\n");debugMessage(debugMsg, par);


	while (pointer!=NULL)
	{
		printCandidate(pointer, par);
		//printf("%lf\n", pointer->deltaFO);

		pointer = pointer->next;
	}
	sprintf(debugMsg, "\n");debugMessage(debugMsg, par);
}

/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */

void printCandidate(sCandidates *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	printApplication(pointer->app_i, par);
	printApplication(pointer->app_j, par);
    sprintf(debugMsg, "newCoresAssignment1 = %d newCoresAssignment2 = %d Totdelta = %lf delta1 = %d delta2 = %d\n\n ",
    		pointer->newCoreAssignment_i, pointer->newCoreAssignment_j,
			pointer->deltaFO, pointer->delta_i, pointer->delta_j);debugMessage(debugMsg, par);
}




