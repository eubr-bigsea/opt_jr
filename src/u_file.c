/*
 * u_file.c
 *
 *  Created on: Oct 31, 2017
 *      Author: work
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


#include "common.h"


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

char * readFolder(char *  path)
{
	 struct dirent *de;

	    DIR *dr = opendir(path);

	    if (dr == NULL)
	    {
	        printf("readFolder failure (%s)", path );
	        exit(-1);
	    }

	    while ((de = readdir(dr)) != NULL)
	    {
	    	if (de->d_type == 4) // folder
	    	{
	    		if (
	    				strcmp(de->d_name, ".") != 0 &&
						strcmp(de->d_name, "..")
	    			)
	    		return de->d_name;
	    	}
	    }
	    closedir(dr);
	    return NULL;
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

