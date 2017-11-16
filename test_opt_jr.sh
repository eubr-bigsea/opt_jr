#!/bin/bash
abs_script=`readlink -e $0`
dir_script=`dirname $abs_script`
DB_IP=${DB_IP:-"localhost"}
WSI_IP=${WSI_IP:-"localhost"}
FILE=${FILE:-~/wsi_config.xml}
if [ -f $FILE ]; then
	WSI_PORT="8080" 
        #
	# Get User
        #
        temp=$(cat ${FILE}|grep DB_user)
        USER=$(echo $temp| awk -v FS="(>|<)" '{print $3}')
        #
        # Get password
        #
        temp=$(cat ${FILE}|grep DB_pass)
        PASSWORD=$(echo $temp| awk -v FS="(>|<)" '{print $3}')
        #
        # Get DBNAME
        #
        temp=$(cat ${FILE}|grep DB_dbName)
        DB=$(echo $temp| awk -v FS="(>|<)" '{print $3}')
	# 
	# Build temp mysql configuraton file
	#
	TEMPFILE=/tmp/config.cnf
	echo > $TEMPFILE
	echo "[client]" > $TEMPFILE
	echo "user=$USER" >> $TEMPFILE
	echo "password=$PASSWORD" >> $TEMPFILE
	echo "host=$DB_IP" >> $TEMPFILE 
	echo "database=$DB" >> $TEMPFILE
	APP_IDS=("application_1483347394756_0" \
             "application_1483347394756_1" \
             "application_1483347394756_2" \
             "application_1483347394756_3")
	NS=("150" "200" "220" "150")
	STAGES=("J4S5" "J0S0" "J2S2" "J1S3")

	# Open new session
	echo "Open a new session..."
	SID=$(curl "http://${WSI_IP}:${WSI_PORT}/WSI/session/new")
	echo "Session ID ${SID}"


	# Set the number of calls
	NUMCALLS=${#APP_IDS[@]}
	echo "Setting ${NUMCALLS} calls "
	curl -X POST "http://${WSI_IP}:${WSI_PORT}/WSI/session/setcalls?SID=${SID}&ncalls=${NUMCALLS}&ncores=${NUM_CORES}"
echo ""

	COUNTER=0
	while [  $COUNTER -lt ${NUMCALLS} ]; do
		echo Setting ${APP_IDS[COUNTER]}
    		weight=$(${dir_script}/execute.sh ${APP_IDS[COUNTER]}|awk '{print $1}')
    		deadline=$(${dir_script}/execute.sh ${APP_IDS[COUNTER]}|awk '{print $2}')
    		curl -X POST -H "Content-Type: text/plain" -d "${APP_IDS[COUNTER]} $weight $deadline ${STAGES[COUNTER]} ${NS[COUNTER]}" "http://${WSI_IP}:${WSI_PORT}/WSI/session/setparams?SID=${SID}"	
    		let COUNTER=COUNTER+1 
		echo ""
	done
else
   echo "FATAL ERROR: File $FILE does not exist."
fi

