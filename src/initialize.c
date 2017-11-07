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

#include "initialize.h"




/*
 * Name: fixInitialSolution
 * Input parameters: sApplication *applications,  struct optJrParameters par
 * Output parameters: sApplicationPointers *
 * Description: It fixes the initial solution by reallocating the residual cores to the applications that may need more resources
 */

sApplicationPointers * fixInitialSolution(sApplication *applications,  struct optJrParameters par)
{
	sApplication * first;
	int allocatedCores;
	sApplicationPointers * first_LP = NULL;
	int loopExit = 0;
	sApplicationPointers *CandidatePointer;
	int residualCores;
	char debugMsg[DEBUG_MSG];
	int N = par.number;

	allocatedCores = 0;

	first = applications;

	while (first != NULL)
	{
		//int currentcores1 =first->currentCores_d;
		//double currentcores2=max(((int)(first->currentCores_d / first->V)) * first->V,first->V);

		first->currentCores_d = max(((int)(first->currentCores_d / first->V)) * first->V,first->V);
		if (first->currentCores_d > first->bound)
			first->currentCores_d = first->bound;
		else
			{
				sprintf(debugMsg, "adding %s to ApplicationPointers\n", first->session_app_id);debugMessage(debugMsg, par);
				addApplicationPointer(&first_LP, first);
			}

		// Danilo Application (suffering) insert in the new Application
		// TODO Handle insert in such a way the Application is sorted by weight -> DONE
		allocatedCores+= first->currentCores_d;
		sprintf(debugMsg, "fixInitialSolution FIXING CORES %s %d\n", first->session_app_id, first->currentCores_d);debugMessage(debugMsg, par);
		first = first->next;
	}
	//readApplicationPointers(first_LP);
	sprintf(debugMsg,"fixInitialSolution: allocatedCores %d\n", allocatedCores);debugMessage(debugMsg, par);

	CandidatePointer = first_LP;
	residualCores = N - allocatedCores;
	int addedCores;

	while (!loopExit&& (residualCores>0))
	{
		if (CandidatePointer == NULL) loopExit = 1;
		else
		{
			// cores assignment

			int potentialDeltaCores=((int)(residualCores / CandidatePointer->app->V) )* CandidatePointer->app->V;

			//addedCores = MIN(, CandidatePointer->app->bound_d);

			if ((CandidatePointer->app->currentCores_d + potentialDeltaCores) > CandidatePointer->app->bound){
				addedCores = CandidatePointer->app->bound - CandidatePointer->app->currentCores_d ;
				CandidatePointer->app->currentCores_d = CandidatePointer->app->bound;


			}
			else{
				CandidatePointer->app->currentCores_d = CandidatePointer->app->currentCores_d + potentialDeltaCores;
				addedCores=potentialDeltaCores;
			}

			if (CandidatePointer->app->currentCores_d == 0)
			{
				printf("\nFatal Error: FixInitialSolution: app %s has %d cores after fix\n", CandidatePointer->app->session_app_id, CandidatePointer->app->currentCores_d);
				exit(-1);
			}
			if (addedCores > 0)
			{
				//CandidatePointer->app->currentCores_d+= addedCores;

				sprintf(debugMsg,"adding cores to App %s, %d \n", CandidatePointer->app->session_app_id, addedCores);debugMessage(debugMsg, par);
				sprintf(debugMsg," application_id %s new cores %d moved cores %d\n", CandidatePointer->app->session_app_id, (int)CandidatePointer->app->currentCores_d, addedCores);debugMessage(debugMsg, par);
				residualCores = residualCores - addedCores;
			}
			CandidatePointer = CandidatePointer->next;
		}

		if (residualCores == 0) loopExit = 1;
	}

	return first_LP;
}


/*
 * Name:				initialize
 * Input parameters: 	sConfiguration * configuration, MYSQL *conn, sApplication * application_i, struct optJrParameters par
 * Output parameters:
 * Description			For each application, a base value for the objective function is calculated.
 */

void initialize(sConfiguration * configuration, MYSQL *conn, sApplication * application_i, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "Information: INITIALIZE baseFo for all the applications\n");debugInformational(debugMsg, par);
	while (application_i != NULL)
	{

			application_i->mode = R_ALGORITHM;
			application_i->baseFO = ObjFunctionComponent(configuration, conn, application_i, par);
			application_i->initialBaseFO = application_i->baseFO;
			sprintf(debugMsg,"INITIALIZE BASE FO for APP %s baseFO =%lf", application_i->session_app_id, application_i->baseFO);debugMessage(debugMsg, par);
			application_i = application_i->next;
	}
}

