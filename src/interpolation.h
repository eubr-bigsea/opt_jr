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
