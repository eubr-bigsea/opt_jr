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


#ifndef SRC_BOUNDSCALCULATION_H_
#define SRC_BOUNDSCALCULATION_H_

#include "interpolation.h"
#include "common.h"

void  calculate_Nu(sConfiguration *, MYSQL *, sApplication *,  struct optJrParameters);
float computeBeta(sAlphaBetaManagement );
float computeAlpha(sAlphaBetaManagement , float );

void debugBanner(char * string, struct optJrParameters par);
void debugMessage(char * string, struct optJrParameters par);
void debugInformational(char * string, struct optJrParameters par);
int doubleCompare(double, double);

double getCsi(double , double );

char* invokePredictor(sConfiguration * , MYSQL *, int , int , char * , int ,  char *, char *, char *, struct optJrParameters, int);

double max(double, double);

void printApplications(sApplication *, struct optJrParameters);
#endif /* SRC_BOUNDSCALCULATION_H_ */
