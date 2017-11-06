/*
 * interpolation.c
 *
 *  Created on: Oct 31, 2017
 *      Author: work
 */

#include "common.h"



/*
 * Name: approximatedLoop
 * Input parameters: sList *first_i, int *iteration, struct optJrParameters par
 * Output parameters: sCandidates *
 * Description: It estimates the objective function for each move. The candidate applications for which the move is profitable is stored in an Candidateiliary list
 */
sCandidates * approximatedLoop(sApplication *first_i, int *iteration, struct optJrParameters par )
{
	char debugMsg[DEBUG_MSG];

	if (first_i == NULL)
	{
		printf("Error: approximatedLoop: null pointer\n");
		exit(-1);
	}

	int record = 0;
	int nCoreMov;
	double DELTAVM_i;
	double DELTAVM_j;
	double DELTA_fo_App_i, DELTA_fo_App_j;
	sCandidates * scurrentCandidateApproximated = NULL;
	sCandidates * sfirstCandidateApproximated = NULL;
	sApplication *application_i,  *application_j;


	/* Initialize to the first element */
	/* (possibly redundant) */
	application_i = first_i;

	sprintf(debugMsg, "Approximated iterated loop");debugInformational(debugMsg, par);
	while (application_i != NULL)
	{
		application_j = first_i;
		while (application_j != NULL)
		{
			if (strcmp(application_i->session_app_id, application_j->session_app_id)!= 0)
			{
				sprintf(debugMsg,"Comparing %s with %s", application_i->session_app_id, application_j->session_app_id);debugMessage(debugMsg, par);

				nCoreMov = max(application_i->V, application_j->V);

				DELTAVM_i = nCoreMov/application_i->V;sprintf(debugMsg, "app %s DELTAVM_i %lf", application_i->session_app_id, DELTAVM_i);debugMessage(debugMsg, par);
				DELTAVM_j = nCoreMov/application_j->V;sprintf(debugMsg, "app %s DELTAVM_j %lf", application_j->session_app_id, DELTAVM_j);debugMessage(debugMsg, par);

				/* Change the currentCores, but rollback later */
				int deltaNCores_i = DELTAVM_i * application_i->V;
				int deltaNCores_j = DELTAVM_j * application_j->V;
				application_i->currentCores_d = application_i->currentCores_d + deltaNCores_i;
				application_j->currentCores_d = application_j->currentCores_d - deltaNCores_j;

				sprintf(debugMsg,"After cores exchange: app %s currentCores %d", application_i->session_app_id, (int)application_i->currentCores_d);debugMessage(debugMsg, par);
				sprintf(debugMsg,"After cores exchange: app %s currentCores %d", application_j->session_app_id, (int)application_j->currentCores_d);debugMessage(debugMsg, par);


				if (application_i->currentCores_d > 0 && application_j->currentCores_d > 0)
				{
					DELTA_fo_App_i = ObjFunctionComponentApprox(application_i, par) - application_i->baseFO;
					sprintf(debugMsg,"app %s DELTA_fo_App_i %lf\n", application_i->session_app_id, DELTA_fo_App_i);debugMessage(debugMsg, par);

					DELTA_fo_App_j = ObjFunctionComponentApprox(application_j, par) - application_j->baseFO;
					sprintf(debugMsg,"app %s DELTA_fo_App_j %lf", application_j->session_app_id, DELTA_fo_App_j);debugMessage(debugMsg, par);

					if ((DELTA_fo_App_i + DELTA_fo_App_j < 0))
					{
						addCandidate(&sfirstCandidateApproximated,
									&scurrentCandidateApproximated,
									application_i,
									application_j,
									application_i->currentCores_d,
									application_j->currentCores_d,
									DELTA_fo_App_i + DELTA_fo_App_j,
									DELTAVM_i,
									DELTAVM_j
									);
							/* Update the number of insertions */
						++record;
					}


				}
				// TODO DANILO Restore previous number of cores -> ENRICO DONE
				application_i->currentCores_d = application_i->currentCores_d - DELTAVM_i*application_i->V;
				application_j->currentCores_d = application_j->currentCores_d + DELTAVM_j*application_j->V;
			}
		application_j = application_j->next;
		}
	application_i = application_i->next;
	}

	*iteration = record;

	return sfirstCandidateApproximated;
}


/*
 * Name: computeAlpha
 * Input parameters: sAlphaBetaManagement sab, float Beta
 * Output parameters: float
 * Description: It calculates the second parameter used by the interpolation algorithm
 */
float computeAlpha(sAlphaBetaManagement sab, float Beta)
{
	return sab.vec[1].nCores * (sab.vec[1].R - Beta);
}

/*
 * Name: computeBeta
 * Input parameters: sAlphaBetaManagement sAB
 * Output parameters: float
 * Description: It calculates the first parameter used by the interpolation algorithm
 */
float computeBeta(sAlphaBetaManagement sAB)
{
	return ((double) sAB.vec[1].nCores) / (sAB.vec[0].nCores - sAB.vec[1].nCores) * (((double) sAB.vec[0].nCores)/sAB.vec[1].nCores * sAB.vec[0].R - sAB.vec[1].R);
}



