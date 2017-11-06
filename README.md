OPT_JR is the application balancer used within the BIG SEA project. It provides the best cores allocation to the invoking applications by performing a local neighborhood search. OPT_JR uses a predictor (dagSim) and relies on the initial solution provided by OPT_IC.

Usage:

./OPT_JR -f <filename.csv> -n <N> -k <Limit> -d <Y/y|N/n> -c <Y/y|N/n> -g=y -s <dagSim|lundstrom> -i <iterations> -t <threads_number>

where:

<filename.csv> is the csv file defining the profile applications under $UPLOAD_HOME in wsi_config.xml;
<N> is the total number of cores;
<Limit> is the maximum number of considered candidates (if equal to 0, all the candidates are considered).
-d represents debug (on/off)
-c represents cache (on/off)
-g prints the value of the global objective function (on/off)
-i represents the maximum number of iterations
-t represents the number of threads (if 0, then multi-threading won't be taken in account)

Example:
-f="Test1.csv" -n=150  -i=10 -k=0 -d=y -c=y -s=dagSim -g=Y -t=4

In this case, OPT_JR is executed on a file Test1.csv, the total number of cores is 150, all the candidates are considered, the predictor used is dagSim, the maximum number of iterations is 10, the value of global objective function for each iteration will be printed in output and 4 threads are considered for multi-threading.

