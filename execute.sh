parameter=$1
mysql --defaults-extra-file=config.cnf -N -B -A -e "select weight, deadline from RUNNING_APPLICATION_TABLE where application_session_id='${parameter}';"
 
