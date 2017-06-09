/*
## Copyright 2017 Enrico Barbierato  <enrico.barbierato@polimi.it>
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


#include <stdio.h>
#include <string.h>
#include "db.h"

/*
 * Standard error procedure for DB operations
 */
void DBerror(MYSQL *conn, char * msg)
{
	printf("DBError: %s %s\n", msg, mysql_error(conn));
	mysql_close(conn);
	exit(-1);
}

/*
 * Open a DB connection
 */
MYSQL * DBopen(char * host, char * login, char * passw, char *dbName)
{


MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
      fprintf(stderr, "DBError: %s\n", mysql_error(con));
      exit(1);
  }

  if (mysql_real_connect(con, host, login, passw,
          dbName, 0, NULL, 0) == NULL) DBerror(con, "mysql_real_connect");

  return con;
}

/*
 * Close the Db connection
 */
void DBclose(MYSQL *conn)
{
	mysql_close(conn);
}

/*
 * Insert a row in opt table
 */
void DBinsertrow(MYSQL * conn, char *id, char *app_id, float nu)
{
	char * values = (char *)malloc(2048);
	char nuS[__SIZEOF_FLOAT__];

	strcpy(values, "INSERT INTO OPT_SESSIONS_RESULTS VALUES('");
	strcat(values, id);
	strcat(values, "','");
	strcat(values, app_id);
	strcat(values, "',");
	sprintf(nuS, "%g", nu);
	strcat(values, nuS);
	strcat(values, ");");


	if (mysql_query(conn, values))
		DBerror(conn, "DBinsertrow failure");

}
