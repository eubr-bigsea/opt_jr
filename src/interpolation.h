/*
 * interpolation.h
 *
 *  Created on: Oct 31, 2017
 *      Author: work
 */

#ifndef SRC_INTERPOLATION_H_
#define SRC_INTERPOLATION_H_

#define HYP_INTERPOLATION_POINTS  2
/*
 * struct lastSimulatorRun
 * Description List used for interpolation
 */
struct lastSimulatorRun
{
	int nCores;
	double R;

};
typedef struct lastSimulatorRun slastSimulatorRun;

/*
 * struct AlphaBetaManagement
 * Description List used for interpolation
 */
struct AlphaBetaManagement
{
	slastSimulatorRun vec[HYP_INTERPOLATION_POINTS];
	int index;
};
typedef struct AlphaBetaManagement sAlphaBetaManagement;


#endif /* SRC_INTERPOLATION_H_ */
