
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>
#include <sys/time.h>

#include "boundsCalculation.h"

/*
 * 		Name:				Bound
 * 		Input parameters:	int deadline, int nNodes, int nCores, int datasetSize, char *appId,
 * 		Output parameters:	int *R (initial Predictor estimate for the given cores, the bound (number of cores), and the time for the determined bound.
 * 		Description:		This function calculates the bound given a certain deadline and number of nodes, cores. Predictor method is invoked until an upper bound,
 * 							consisting of the number of nodes, is found (once that the time calculated by the predictor, a rollback is performed to
 * 							return the last "safe" number of core and time.
 */
void  Bound(sConfiguration *configuration, MYSQL *conn, sApplication * pointer, struct optJrParameters par)
{
	double predictorOutput;
	char debugMsg[DEBUG_MSG];
	double BTime = 0;
	int BCores = 0;
	int STEP = pointer->V;
	int nCores;
	int nNodes = 1; // Temporary fix

	pointer->currentCores_d = pointer->nCores_DB_d;

	/* Perform adjustment */
	int X0 = ((int) ( pointer->currentCores_d / pointer->V) ) * pointer->V;
	pointer->currentCores_d = max( X0, pointer->V);

	nCores = pointer->currentCores_d;
	predictorOutput = atoi(invokePredictor(configuration, conn, nNodes, nCores, "*", pointer->datasetSize, pointer->session_app_id,
							pointer->app_id, pointer->stage, par, WHOLE_EXECUTION_TIME));
	sprintf(debugMsg,"Bound evaluation for %s predictorOutput = %lf (deadline is %lf) cores %d\n",  pointer->session_app_id, predictorOutput, pointer->Deadline_d, nCores);debugMessage(debugMsg, par);
	// Danilo 27/7/2017
		pointer->sAB.index = 0;
		pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
		pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
		pointer->sAB.index++;
	// End Danilo


	BTime = predictorOutput;
	//To do nCores = ceil(nCores / pointer->V ) * pointer->V
	//printf("Calculate Bound for %s: R %d D %d\n", appId, predictorOutput, deadline);
	if (doubleCompare(predictorOutput, pointer->Deadline_d) == 1)
		while (predictorOutput > pointer->Deadline_d)
		{
			if (nCores ==0)
			{
				printf("Warning Bound (| if case): nCores is currently 0 for app. Cannot invoke Predictor\n");
				nCores= pointer->V;
				//leave the while loop
				break;
			}
			nCores = nCores + STEP;
			predictorOutput = atof(invokePredictor(configuration, conn, nNodes, nCores, "8G", pointer->datasetSize, pointer->session_app_id, pointer->app_id, pointer->stage,par, WHOLE_EXECUTION_TIME));
			sprintf(debugMsg,"Bound evaluation for %s predictorOutput = %lf (deadline is %lf) cores %d\n",  pointer->session_app_id, predictorOutput,pointer->Deadline_d, nCores);debugMessage(debugMsg, par);

			BCores = nCores;
			BTime = predictorOutput;

			// Danilo 27/7/2017
				pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
				pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
				pointer->sAB.index = (pointer->sAB.index +1) % HYP_INTERPOLATION_POINTS;
			// End Danilo
			pointer->boundIterations++;
		}
		else
			while (doubleCompare(predictorOutput, pointer->Deadline_d) == -1)
			{
				BCores = nCores;
				BTime = predictorOutput;
				nCores = nCores - STEP;
				if (nCores <0)
				{
					printf("nCores is currently 0. Cannot invoke Predictor\n");
					exit(-1);
				}
				if (nCores == 0)
				{
					//printf("Warning Bound (< if case): nCores is currently 0 for app. Cannot invoke Predictor\n");
					nCores= pointer->V;
					//leave the while loop
					break;
				}
				predictorOutput = atof(invokePredictor(configuration, conn, nNodes, nCores, "8G", pointer->datasetSize, pointer->session_app_id, pointer->app_id, pointer->stage, par, WHOLE_EXECUTION_TIME));
				sprintf(debugMsg,"Bound evaluation for %s predictorOutput = %lf (deadline is %lf) cores %d\n",  pointer->session_app_id, predictorOutput, pointer->Deadline_d, nCores);debugMessage(debugMsg, par);

				pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
				pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
				pointer->sAB.index = pointer->sAB.index % HYP_INTERPOLATION_POINTS;

				//printf("(down) time = %d Rnew =%d\n", time, BTime);
				pointer->boundIterations++;
			}
	//}

	/* Update the record with bound values */
	pointer->currentCores_d = BCores;
	pointer->R_d = BTime;
	pointer->bound = BCores;

	sprintf(debugMsg,"\n\nSession_app_id %s APP_ID %s D = %lf R = %lf  bound = %d\n\n", pointer->session_app_id, pointer->session_app_id, pointer->Deadline_d, pointer->R_d, pointer->bound);debugMessage(debugMsg, par);
}




/*
 * 		Name:					calculate_Nu
 * 		Input parameters:		MYSQL *conn, char * insertuniqueFilename, sApplication *current, double nu_1, double w1, double csi_1, double chi_c_1
 * 		Output parameters:		none
 * 		Description:			Given nu_1 and other measures related to the first applications:
 * 								- it computes the nu indices for all the other applications:
 * 								- it updates the DB;
 * 								- it calculates the bound for each application
 *
 */
void calculate_Nu(sConfiguration * configuration, MYSQL *conn, sApplication *first, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	sApplication * current = first;
	int rows = 0;
	char * app_id;
	int w1;
	double chi_c_1;
	double csi_1;
	int N = par.number;
	double csi;

	int minCapacity= 0;


	sprintf(debugMsg, "Calculate nu indices for each application");debugBanner(debugMsg, par);

	//TO DO sum di V_i -> DONE
	while (current != NULL)
	{
		minCapacity+= current->V;
		current = current->next;
	}
	current = first;

	N = N - minCapacity;

	if (current == NULL)
	{
		printf("Fatal error: calculate_nu: current cannot be NULL\n");
		exit(-1);
	}

	app_id = (char *)malloc(MAX_APP_LENGTH);
	if (app_id == NULL)
	{
	          printf("app_id: malloc_failure in process\n");
	          exit(-1);
    }


	/* Calculate nu_1 */
	double tot = 0;
	while (current != NULL)
	{
		if (rows == 0) /* First row only */
		{
			w1 = current->w;
			chi_c_1 = current->chi_C;
			csi_1 = getCsi(current->M/current->m, current->V/current->v);
			//printf("Calculate_nu first app: %s w1 %d chi_c %lf chi_c_1%lf\n", current->app_id, w1, chi_c_1, csi_1);
		}
		else /*Any other row */
		{
		       csi = getCsi(current->M/current->m, current->V/current->v);
		       current->term_i = sqrt((current->w/w1)*(current->chi_C/chi_c_1)*(csi_1/csi));
		       tot = tot + current->term_i;
		      // printf("Calculate_nu  Other rows: %s w %d csi %lf tot %lf\n", current->app_id, current->w, csi, tot);
		}
		rows++;
		current = current->next;
	}

	double nu_1 = N/(1 + tot);
	//printf("nu_1=%lf\n", nu_1);

	rows = 0;
	tot = 0;
	current = first;
	double term_j;

	while (current != NULL)
	{
		//findBound(configuration, conn, getConfigurationValue(configuration, "OptDB_dbName"), current, par);
		if (rows > 0)
		{
			csi = getCsi(current->M/current->m, current->V/current->v);
			term_j = sqrt((current->w/w1)*(current->chi_C/chi_c_1)*(csi_1/csi));
			tot = tot+ term_j;

		} else rows++;
	    current = current->next;
	}

	rows = 0;
	    while (first != NULL)
	    {
	    	if (rows == 0) first->nu_d = nu_1;
	    	else
	    	{
	    		first->nu_d = (first->term_i/(1 + tot))*N;
	    		//printf("\nTERM app %s %lf tot %lf\n", first->app_id, first->term_i, (1 + tot) );
	    	}
	    	//printf("NU_i%lf nu1 %lf\n", first->nu_d, nu_1);
	    	first->currentCores_d = first->nu_d;



	    	first->beta = computeBeta(first->sAB);
	    	first->alpha = computeAlpha(first->sAB, first->beta);

	        //printf("App %s alpha = %lf beta = %lf\n", first->app_id, first->alpha, first->beta);
	        //printf(" R %lf nCores %d\n", first->sAB.vec[0].R, first->sAB.vec[0].nCores);
	        //printf(" R %lf nCores %d\n", first->sAB.vec[1].R, first->sAB.vec[1].nCores);

	        //current->currentCores_d = current->bound_d;

	        printApplication(first, par);

	        first = first->next;
	        rows++;
	   }
	    sprintf(debugMsg,"end calculate nu");debugBanner(debugMsg, par);
}

/*
 * Name:
 * Input parameters:
 * Output parameters:
 * Description
*/



void calculateBounds(sApplication * pointer,  sConfiguration * configuration, MYSQL *conn, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	char statement[256];

	debugBanner("calculateBounds", par);

	while (pointer != NULL)
	{

		/* Retrieve the value calculated by OPT_IC from DB */
		sprintf(debugMsg,"\n calculateBounds %s %s\n", pointer->session_app_id, pointer->app_id);debugMessage(debugMsg, par);
		//Retrieve nCores from the DB
		sprintf(statement,
                        "select num_cores_opt, num_vm_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%lf;"
				, getConfigurationValue(configuration, "OptDB_dbName"), pointer->app_id, pointer->datasetSize, pointer->Deadline_d);

		MYSQL_ROW row = executeSQL(conn, statement, par);
		if (row == NULL)
		{
			printf("Fatal error: no matches found on OPTIMIZER_CONFIGURATION_TABLE.\nCHeck that OPT_IC was run over the specific application.\n");
			exit(-1);
		}

		pointer->nCores_DB_d = atoi(row[0]);
		pointer->vm = atoi(row[1]);

		Bound(configuration, conn, pointer, par);
		sprintf(debugMsg,"A bound for %s has been calculated", pointer->session_app_id);
		debugMessage(debugMsg, par);

		pointer = pointer->next;
	}

}



