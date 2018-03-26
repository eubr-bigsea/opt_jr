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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <glob.h>


#include "u_file.h"


void updateLundstromConfig(sConfiguration *configuration, char * path, int mode)
{
	char content[1024], configfile[1024];
	FILE *f;

	getConfigurationValue(configuration, "LUNDSTROM_HOME");
	sprintf(content, "{\n\t \"SPARK_LOG_DIR\": \"%s/"
			"%%NODES_%%CORES_%%RAM_%%DATA/%%QUERY/logs/\",\n\t \"COMPSS_LOG_DIR\": \"/home/disk2/speed/tuliobraga/Downloads/compss-daniele/%%QUERY/%%NODES_%%CORES_%%RAM_%%DATA/\"\n}",
			getConfigurationValue(configuration, path));

	sprintf(configfile, "%s/config.json", getConfigurationValue(configuration, "LUNDSTROM_HOME"));

	switch(mode)
	{
		case SINGLE_THREAD:
			f = fopen(configfile, "w");
			if (f == NULL)
			{
			    printf("Error opening %s file!\n", configfile);
			    exit(-1);
			}
			fprintf(f, "%s", content);
			fclose(f);
			break;
		case MULTI_THREAD:
			updateLundstromConfigMP(configfile, content);
			break;
	}


}


char * setLuaFilename(sApplication *application, sConfiguration * configuration, char *appId, struct optJrParameters par,  char * dataset )
{

	char path[1024];
	char subfolder[1024];
	char string[1024];
	char *output = (char *)malloc(1024);

	strcpy(path, getConfigurationValue(configuration, "RESULTS_HOME"));
	strcpy(application->results, "RESULTS_HOME");
	/* Determine the data log folder for the predictor */
	/* 1 means that there will be a test regarding subfolders starting like XX_ or X_ */
	strcpy(subfolder, readFolder(path, 1));
	if (strstr(subfolder, dataset) == NULL)
		/* Look for the alternative log folder */
	{
		strcpy(path, getConfigurationValue(configuration, "ALTERNATIVE_RESULTS_HOME"));
		strcpy(application->results, "ALTERNATIVE_RESULTS_HOME");
		strcpy(subfolder, readFolder(path, 1));
	}
        printf("Subfolder which will be used is %s\n", subfolder);
        

	/* Check if the filename path is rthe right one, i.e. includes the dataset */

	sprintf(path, "%s/%s/%s/logs/", path, subfolder, appId);
	sprintf(string, "%s/%s/*.lua", path, readFolder(path, 0) );

	strcpy(output, ls(string, par));
	return output;
}

char * ls(char * pattern, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	glob_t pglob;


	char *filename = (char *)malloc(1024);
	 int outcome = glob(pattern, GLOB_ERR, NULL, &pglob);

	 if (pglob.gl_pathc == 1)
	 	  {
	 		  sprintf(debugMsg, "ls: %s\n", pglob.gl_pathv[0]);debugMessage(debugMsg, par);
	 		  strcpy(filename, pglob.gl_pathv[0]);
	 		  globfree(&pglob);
	 		  return filename;
	 	  }
	 	  else
	 	  {
	 		  switch(outcome)
	 		  {
	 		  	  case GLOB_NOSPACE:
	               printf("Fatal error: ls: running out of memory: %s\n", pattern);
	               exit(-1);
	 	 	 	   break;
	 		  	  case GLOB_ABORTED:
	               printf("Fatal error: ls: a read error has occurred%s\n", pattern);
	               exit(-1);
	               break;
	 		  	  case GLOB_NOMATCH:
	               printf("Fatal error: ls: no matches found%s\n", pattern);
	               exit(-1);
	 		  	  default:
	    	  	   printf("Fatal error: ls: unknown error%s\n", pattern);
	    	  	   exit(-1);
	 		  }
	 	  }
	  return NULL;
}

/*
 * 		Name:					readFolder
 * 		Input parameters:		A path to a folder
 * 		Output parameters:		The name of subfolder contained in the folder corresponding to the folder in "path"
 * 		Description:			This function returns, the first subFolder in the folder corresponding to "path"
 *
 */

char * readFolder(char *  path, int test)
{
	 struct dirent *de;
	 char *foldername = (char *)malloc(512);
	 char *string = (char*)malloc(512);
	 int initial = 0;
	 DIR *dr = opendir(path);

	    if (dr == NULL)
	    {
	        printf("readFolder failure (%s)", path );
	        exit(-1);
	    }
	    strcpy(foldername, "9999999");

	    while ((de = readdir(dr)) != NULL)
	    {
	    	if (de->d_type == 4) // folder
	    	{
	    		if (
	    				strcmp(de->d_name, ".") != 0 &&
						strcmp(de->d_name, "..")
	    			)
	    		{
				if (initial == 0)
				{
					initial = 1;
					strcpy(foldername, de->d_name);
				}	

	    			/* Check if the name has the patter X_ or XX_: if _X, then add a "0" */
				if (test == 1)
	    			if (strlen(de->d_name) - strlen(strstr(de->d_name, "_")) == 1)
	    			{
	    				strcpy(string, "0");
	    				strcat(string, de->d_name);
	    				strcpy(de->d_name, string);
                                 
	    			}

	    		 if (strcmp(de->d_name, foldername) < 0)
	    			 strcpy(foldername, de->d_name); 
	    		}
	    	}
	    }



	    closedir(dr);

	    /* if a "0" was added, then remove it */
	    int pos;
	    if (test == 1)
	    if (foldername[0] == '0')
	    {
	    	for (pos = 1; pos < strlen(foldername); pos++)
                   string[pos-1] = foldername[pos];
	    	string[pos-1] = '\0';
	    	strcpy(foldername, string);
	    }

	    return foldername;
}


void writeFile(const char *filepath, const char *data)
{
    FILE *fp = fopen(filepath, "w");

    if (fp != NULL)
    {
        fputs(data, fp);
        fclose(fp);
    }
}

char * readFile(char * filename)
{
	   struct stat sb;
	   stat(filename, &sb);
	   FILE *fp=fopen(filename, "r");

	   if (fp == NULL)
	   {
		   printf("Fatal error: readFile: could not open %s\n", filename);
		   exit(-1);
	   }

	   char *str=malloc(sb.st_size+1);

	   if (str == NULL)
	   {
		   printf("Malloc failure: readFile\n");
		   exit(-1);
	   }

	   fread(str, 1, sb.st_size, fp);
	   fclose(fp);
	   str[sb.st_size]=0;
	   return str;

}


int read_line(FILE *in, char *buffer, size_t max)
{
  return fgets(buffer, max, in) == buffer;
}

