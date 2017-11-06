/*
 * boundsCalculation.h
 *
 *  Created on: Nov 6, 2017
 *      Author: work
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
