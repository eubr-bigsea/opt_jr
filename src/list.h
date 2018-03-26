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

#ifndef LIST_H_
#define LIST_H_
#include "interpolation.h"



/*
 * List name optJrParameters
 * Description This list includes all the input parameters passed to OPT_JR from the command line
 */
struct optJrParameters
{
	char filename[1024];	/* The csv file */
	int debug;				/* debug option: "y" prints every message, "n" only prints fatal errors */
	int cache;				/* cache option: "y" makes use of the DB predictor cache table; "n" doesn't */
	int globalFOcalculation;/* global FO calculation: "y" calculates at each loop of localSearch function the global objective function value, "n" doesn't */
	int K;					/* Maximum depth: the search of candidates in the auxiliary list stops if this limit is exceeded */
	int predictor;			/* The predictor type: either dagSim or Lundstrom */
	int number;				/* Number of toal cores available for the applications (N) */
	int maxIterations;		/* The maximum number of iterations in LocalSearch */
	int numberOfThreads;	/* The number of MPI threads */
	char luafile[1024];
};


/*
 * List Configuration
 * Description It includes all the variables in the wsi_config.xml file
 */
struct Configuration
{
	char * variable; /* Variable name */
	char *value;	/* Variable's value) */
	struct Configuration *next;
};
typedef struct Configuration sConfiguration;

/*
 * List Application
 * Description profile parameters related to the single application
 */
struct Application
{
	/* Static parameters */
	char results[128];		/* HOME_RESULTS or ALTERNATIVE_HOME_RESULTS */
	int mode; 				/* How the objective function is calculated (currently redundant) */
	char * session_app_id;	/* Session identifier */
	char * app_id;			/* Application identifier */
    double w;					/* Weight  */
    double term_i;			/* Used to calculate nu index */
    double chi_0;			/* ML parameter */
    double chi_C;			/* ML parameter */
    double m;
    double M;
    double V;
    double v;
    double Deadline_d;
    double csi;
    char * stage;			/* Application's stage (used in case of residual time) */
    int datasetSize;
    char luafilename[1024];

    /* Dynamic values */

    double nu_d;				/* nu value */
    int  currentCores_d;		/* Initialized to nu_i */
    int  nCores_DB_d;			/* Initialized to the value from look-up table */
    int bound;					/* Bound (number of cores) */
    double R_d;					/* Value of R as per the predictor */
    double R_bound_d;			/* Bound (R) */
    double baseFO;				/* base FO value (used to calculate the delta) */
    double initialBaseFO;		/* copy of base FO value (used to reset the value) */
    float alpha;				/* First parameter for Hyperbolic interpolation */
    float beta;					/* Second parameter for Hyperbolic interpolation */
    sAlphaBetaManagement sAB;	/* Interpolation */
    int boundIterations;		/* Metrics */
    int vm;						/* Read from OPTIMIZER_CONFIGURATION_TABLE */

	struct Application *next;
};
typedef struct Application sApplication;


// The element corresponding to the event




struct ApplicationPointers
{
	sApplication *app;
	struct ApplicationPointers *next;
};
typedef struct ApplicationPointers sApplicationPointers;


/*
 * List Statistics
 * Description It includes relevant statistica information
 */
struct Statistics
{
	int iteration;		/* Interation number */
	int size;			/* Size of the candidates list */
	double FO_Total;	/* total objective function value for that interation */
	struct Statistics *next;
};
typedef struct Statistics sStatistics;



/*
 * List Candidates
 * Description It includes for each move the information about promising moves
 * (as they are estimated only via interpolations and not computed via the predictor)
 *
 */
struct Candidates
{
	/* First application */
	sApplication * app_i; 		/* Application */
	int newCoreAssignment_i;	/* Application cores after the move */
	int delta_i;				/* Delta cores following the move */
	double real_i;				/* Real predictor value calculated (MPI) after the interpolation */

	/* Second application */
	sApplication * app_j;
	int newCoreAssignment_j;
	int delta_j;
	double real_j;
	int nodes_i, nodes_j;

	double deltaFO;				/* Delta Objective Function following the move */

	struct Candidates *next;
};
typedef struct Candidates sCandidates;


#endif /* LIST_H_ */
