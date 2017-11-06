/*
 * objectiveFunction.c
 *
 *  Created on: Oct 31, 2017
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


/*
 * Name: ObjFunctionGlobal
 * Input parameters: sConfiguration * configuration, MYSQL *conn, sApplication * pointer, struct optJrParameters par
 * Output parameters:double, the total value of objective function
 * Description:It calculates the value of the total objective function
 */
double ObjFunctionGlobal(sConfiguration * configuration, MYSQL *conn, sApplication * pointer, struct optJrParameters par)
{

	double sum = 0;

	if (pointer == NULL)
	{
		printf("Warning: NULL pointer in ObjFunctionGlobal\n");
	}
	while (pointer != NULL)
	{
		sum = sum + ObjFunctionComponent(configuration, conn, pointer, par);
		pointer = pointer->next;
	}

	if (doubleCompare(sum, 0) == 0)
	{
		debugInformational("ObjFunctionGlobal: sum equal to zero", par);
		//exit(-1);
	}

	return sum;
}


/*
 * 		Name:						ObjFunctionComponent
 * 		Input parameters:			The pointer to the applications Application
 * 		Output parameters:			double The contribution to the calculation of the objective function
 * 		Description:				Currently, only one method is supported. Note that the algorithm's choice is stored in the "mode" field
 * 									of the application structure.
 *
 */

double ObjFunctionComponent(sConfiguration *configuration, MYSQL *conn, sApplication * pointer, struct optJrParameters par)
{

	char debugMsg[DEBUG_MSG];
	double output;

	if (pointer == NULL)
	{
		printf("ObjFunctionComponent failure: NULL pointer\n");
		exit(-1);
	}

	/* The memory pattern can be anything such as "*" */
	pointer->R_d = atof(invokePredictor( configuration, conn, 1, pointer->currentCores_d, "*", pointer->datasetSize, pointer->session_app_id, pointer->app_id, pointer->stage, par,WHOLE_EXECUTION_TIME));
	//printf("ObjFunctionComponent: App_id %s w %f R %d D %d nCores %d newCores %d\n",pointer->app_id, pointer->w, pointer->R, pointer->D, pointer->cores, pointer->newCores);

	/* Determine how the obj function needs to be calculated */
	switch(pointer->mode)
	{
		case R_ALGORITHM:
				sprintf(debugMsg,"ObjFunctionComponent W %d R_d %lf D %lf\n", pointer->w, pointer->R_d, pointer->Deadline_d);debugMessage(debugMsg, par);
				if (pointer->R_d > pointer->Deadline_d)
					output = pointer->w * (pointer->R_d - pointer->Deadline_d);
				else output = 0;
				sprintf(debugMsg,"Compute FO for app %s currentCores_d %d  R %lf FO=%lf\n", pointer->session_app_id, (int)pointer->currentCores_d, pointer->R_d, output);debugMessage(debugMsg, par);
			break;
			/*
		case CORES_ALGORITHM:
			printf("Cores Algorithm\n");
				if (pointer->currentCores > pointer->newCores) output = 0;
				else output = pointer->w * (pointer->Rnew - pointer->D);

			break;
		case NCORES_ALGORITHM:
			printf("NCores Algorithm\n");
				if (pointer->newCores >pointer->bound) output = 0;
				else output = pointer->w * pointer->R - pointer->R;
			break;
			*/
		default:
			printf("ObjFunctionComponent: unknown case within Switch statement: mode %d\n", pointer->mode);
			exit(-1);
			break;
	}

	return output;
}

/*
 * Name: ObjFunctionComponentApprox
 * Input parameters: pointer to the applications
 * Output parameters: a double The value of the the approximated objective function
 * Description It computes an approximation of the objective function
 */
double ObjFunctionComponentApprox(sApplication * pointer, struct optJrParameters par)
{

	char debugMsg[DEBUG_MSG];
	double output;

	if (pointer == NULL)
	{
		printf("ObjFunctionComponentApprox failure: NULL pointer\n");
		exit(-1);
	}


	pointer->R_d = pointer->alpha/pointer->currentCores_d + pointer->beta;  //atof(invokePredictor( 1, (int)pointer->currentCores_d, "8G", pointer->datasetSize, pointer->app_id));
	//printf("ObjFunctionComponent: App_id %s w %f R %d D %d nCores %d newCores %d\n",pointer->app_id, pointer->w, pointer->R, pointer->D, pointer->cores, pointer->newCores);

	/* Determine how the obj function needs to be calculated */

	sprintf(debugMsg,"W %d R_d %lf D %lf\n", pointer->w, pointer->R_d, pointer->Deadline_d);debugMessage(debugMsg, par);
	if (pointer->R_d > pointer->Deadline_d)
		output = pointer->w * (pointer->R_d - pointer->Deadline_d);
	else output = 0;
		sprintf(debugMsg,"Compute FO for app %s currentCores_d %d  R %lf FO=%lf\n", pointer->session_app_id, (int)pointer->currentCores_d, pointer->R_d, output);debugMessage(debugMsg, par);

	return output;
}

