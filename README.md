This component re-balances the number of virtual machines/cores necessary to execute soft deadline running applications when heavy load occurs.

No specific configuration is requested, though it is required that the following DB tables have been created:
`OPTIMIZER_CONFIGURATION_TABLE`;
`RUNNING_APPLICATION_TABLE`;
`APPLICATION_PROFILE_TABLE`;
`OPT_SESSIONS_RESULTS`

Usage:;
./optimize -f <filename.csv> -n <N> -k <Limit> -d <dY/y|N/n> -c <Y/y|N/n> -s <dagSim|lundstrom> -i <iterations>

where:

<filename.csv> is the csv file (including the input values) under $UPLOAD_HOME in wsi_config.xml
<N> is the total number of cores
<Limit> is the maximum number of considered candidates (if equal to 0, all the candidates are considered
-d represents debug (on/off)
-i represents the maximum number of iterations

Example:

./OPT_JR -f=\"Test3.csv\" -n=220 -k=0 -d=Y -s=dagSim -i=10"

OPT_JR is executed on a file Test3.csv, the total number of cores is 220, all the candidates are considered, the predictor used is dagSim, and
                        the maximum number of iterations is 10

