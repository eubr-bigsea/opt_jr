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


#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include "db.h"
#include "list.h"



#define DEBUG_MSG 512

/* Number of values in the csv file */
#define PARAMETERS 12

#define R_ALGORITHM 0
#define CORES_ALGORITHM 1
#define NCORES_ALGORITHM 2



#define BIG_LINE 4000
#define BIG_TEXT 20000

#define PRODUCT 2
#define COUPLE 1

#define WHOLE_EXECUTION_TIME 0
#define RESIDUAL_EXECUTION_TIME 1

#define FIRST_APP 0
#define OTHER_APPS 1

#define MAX_APP_LENGTH 1024


#define DAGSIM 0
#define LUNDSTROM 1

#define ARGS 9 // Command line

#define FILENAME "-f="
#define NUM_N "-n="
#define APPLICATIONS_LIMIT "-k="
#define DEBUG "-d="
#define MAX_ITERATIONS "-i="
#define SIMULATOR "-s="
#define GLOBAL_FO_CALCULATION "-g"
#define CACHE "-c"

#define NUMBER 0
#define STRING 1
#define YES_NO 2

#define NO 0
#define YES 1


/* Templates */


void addApplicationPointer(sApplicationPointers ** ,  sApplication *);
void addStatistics(sStatistics ** , sStatistics ** , int , int, double );
void addApplication(sApplication ** ,  sApplication **, char *, char *, double , double  , double , double , double , double , double , double , double, char *, int  );
sCandidates * approximatedLoop(sApplication *, int *, struct optJrParameters );
void addCandidate(sCandidates ** , sCandidates ** ,  sApplication * , sApplication * , int , int , double, double, double);

struct Best bestMatch(char *, int);
void  Bound(sConfiguration *, MYSQL *conn, sApplication *, struct optJrParameters);

void  calculate_Nu(sConfiguration *, MYSQL *, sApplication *,  struct optJrParameters);
void calculateOpenMPBounds(sApplication * pointer, int n_threads, sConfiguration * configuration, MYSQL *conn, struct optJrParameters par);
void calculateBounds(sApplication * pointer, sConfiguration * configuration, MYSQL *conn, struct optJrParameters par);
void checkTotalNodes(int N, sApplication * pointer);

float computeBeta(sAlphaBetaManagement );
float computeAlpha(sAlphaBetaManagement , float );
void commitAssignment(sApplication *, char *,  double, struct optJrParameters );
int checkTotalCores(sApplication * pointer, double N);

void debugBanner(char * string, struct optJrParameters par);
void debugMessage(char * string, struct optJrParameters par);
void debugInformational(char * string, struct optJrParameters par);
int doubleCompare(double, double);
void DBerror(MYSQL *, char * );
MYSQL * DBopen(char * , char * , char *, char * );
void DBclose(MYSQL *conn);
void DBinsertrow(MYSQL * , char *, char *, float );

MYSQL_ROW executeSQL(MYSQL *, char *, struct optJrParameters);

double elapsedTime(struct timeval , struct timeval );
char * extractWord(char * , int );
char * extractItem(const char *const string, const char *const left, const char *const right);
char * extractRowN(char *, int );
char * extractRowMatchingPattern(char *text, char *pattern);

sApplicationPointers * fixInitialSolution(sApplication *applications,  struct optJrParameters);
void findBound(sConfiguration *, MYSQL *conn, char *,  sApplication *, struct optJrParameters);
void freeStatistics(sStatistics * );
void freeParameters(sApplication * pointer);
void freeApplications(sApplicationPointers * pointer);
void freeCandidates(sCandidates * pointer);
sCandidates * findMinDelta(sCandidates * );

void howAmIInvoked(char **, int );

char * getfield(char* , int);
char *getConfigurationValue(sConfiguration *pointer, char * variable);
double getCsi(double , double );

void initialize(sConfiguration * configuration, MYSQL *conn, sApplication * application_i, struct optJrParameters par);
char* invokePredictor(sConfiguration * , MYSQL *, int , int , char * , int ,  char *, char *, char *, struct optJrParameters, int);
void invokePredictorOpenMP(sCandidates *sfirstCandidateApproximated, struct optJrParameters, sConfiguration *configuration);

void localSearch(sConfiguration *, MYSQL *conn, sApplication *, struct optJrParameters);
char * ls(char *, struct optJrParameters);
char * LundstromPredictor(sConfiguration *, int , char * , struct optJrParameters);

double max(double, double);

double ObjFunctionGlobal(sConfiguration *, MYSQL *conn, sApplication *, struct optJrParameters);
double   ObjFunctionComponent(sConfiguration * ,MYSQL *, sApplication *,  struct optJrParameters );
double   ObjFunctionComponentApprox(sApplication *, struct optJrParameters );

char * parseConfigurationFile(char *, int);
sApplication * parseCsv(sConfiguration *configuration, struct optJrParameters par);
struct optJrParameters parseCommandLine(char **args, int argc);
char * parseArg(char * string, char * gap, int type);
void printConfigurationFile(sConfiguration *pointer);

void printApplications(sApplication *, struct optJrParameters);
void printApplication(sApplication *, struct optJrParameters);
void printCandidate(sCandidates *, struct optJrParameters);
void printOPT_JRPars(struct optJrParameters par );

sConfiguration * readConfigurationFile();
void readStatistics(sStatistics *, struct optJrParameters);


void readCandidates(sCandidates *, struct optJrParameters);
void readApplicationPointers(sApplicationPointers *, struct optJrParameters);
char *readFolder(char *);
char * replace(char * , char *);
int read_line(FILE *, char *, size_t );
char * readFile(char * );
void readSolution(sApplication *pointer);
void restoreInitialBaseFO(sApplication * pointer, struct optJrParameters par);
MYSQL_ROW retrieveTimeFromDBCash(MYSQL *conn, char *sessionId, char *appId, int datasize, int ncores );
char * _run(char *,  struct optJrParameters );

void split (char str[], int *a, int *b);
sApplication * searchApplication(sApplication * , char *);
void start(sConfiguration *, int , int , char **, int );

void Usage();

void writeResults(MYSQL *conn, char *,sApplication *, struct optJrParameters);
void writeFile(const char *, const char *);






#endif /* SRC_COMMON_H_ */
