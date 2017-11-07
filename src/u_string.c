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
#include <string.h>
#include "u_string.h"

const int MAX_LINE_LENGTH = 1024;

const int _SESSION_APP_ID = 1;
const int _APP_ID = 2;
const int _W = 3;
const int _CHI_0 = 4;
const int _CHI_C = 5;
const int _M = 6;
const int _m = 7;
const int _V = 8;
const int _v = 9;
const int _D = 10;
const int _St = 11;
const int _Dsz = 12;


sApplication * parseCsv(sConfiguration *configuration, struct optJrParameters par)
{
	double w;
	char line[1024];
	double chi_0;
	double chi_C;
	double m;
	double M;
	double V;
	double v;
	double D;
	double csi;
	char * session_app_id;
	char * app_id;
	char * St;
	int rows = 1;
	int DatasetSize;

	sApplication *first = NULL, *current = NULL;
	/*
	 * Find where the file has been uploaded and determine absolute file path
	 */
	 char *folder = getConfigurationValue(configuration, "UPLOAD_HOME");
	 char *filename = strcat(folder, "/");
	 filename = strcat(folder, par.filename);
	 FILE* stream = fopen(filename, "r");
	 if (stream == NULL)
	 {
		printf("FATAL ERROR: could not find or open %s\n", filename);
	    exit(-1);
	 }
	/*
	 * Initialize Vars
	 */

	    session_app_id = (char *)malloc(MAX_APP_LENGTH);
	    if (session_app_id == NULL)
	    {
	          printf("session_app_id: malloc_failure in main\n");
	          exit(-1);
	    }

	    app_id = (char *)malloc(MAX_APP_LENGTH);
	    if (app_id == NULL)
	    {
	          printf("app_id: malloc_failure in main\n");
	          exit(-1);
	    }

	    St = (char *)malloc(1024);
	        if (St == NULL)
	        {
	            printf("app_id1: malloc_failure in main\n");
	            exit(-1);
	        }

	    /*
	      * Read the file and load all the parameters in a list.
	      * Calculate nu_1
	    */
	    while (fgets(line, MAX_LINE_LENGTH, stream))
	    {
	        char* tmp = strdup(line);

	        if ((strlen(line)==0) || (strstr(line, "#")==NULL)) // Skip if it's comment or empty line
	        {
	        	strcpy(session_app_id, getfield(tmp, _SESSION_APP_ID));tmp = strdup(line);
	        	strcpy(app_id, getfield(tmp, _APP_ID));tmp = strdup(line);
	        	w = 	atof(getfield(tmp, _W));tmp = strdup(line);
	        	chi_0 = atof(getfield(tmp, _CHI_0));tmp = strdup(line);
	        	chi_C = atof(getfield(tmp, _CHI_C));tmp = strdup(line);
	        	M = 	atof(getfield(tmp, _M));tmp = strdup(line);
	        	m = 	atof(getfield(tmp, _m));tmp = strdup(line);
	        	V = 	atof(getfield(tmp, _V));tmp = strdup(line);
	        	v = 	atof(getfield(tmp, _v));tmp = strdup(line);
	        	D = 	atoi(getfield(tmp, _D));tmp = strdup(line);
	        	strcpy(St, getfield(tmp, _St));tmp = strdup(line);
	        	DatasetSize = 	atoi(getfield(tmp, _Dsz));
	        	csi = getCsi(M/m, V/v);
	        	/* Add application parameters to the List */
	        	addApplication(&first, &current, session_app_id, app_id, w, chi_0, chi_C, m, M, V, v, D, csi, St, DatasetSize);
	        	rows++;
	        	free(tmp);
	        }
	    }
	    fclose(stream);
return first;
}

/*
 * 		Name:					getfield
 * 		Input parameters:		char * source, int num
 * 		Output parameters:		A word
 * 		Description:			it extracts a specific value from the csv file
 *
 */

char * getfield(char* line, int num)
{
    char* tok;

    if ((num < 1) || (num > PARAMETERS)) printf("getfield: num %d out of bound\n", num);
    	else for (tok = strtok( line, ","); tok && *tok; tok = strtok(NULL, ",\n"))
    		if (!--num) return tok;

    return NULL;
}

/*
 * Name 		extractItem
 * Input 		const char *const string, const char *const left, const char *const right
 * Output 		char * A word
 * Description:	It returns a word delimited between two characters
 */
char * extractItem(const char *const string, const char *const left, const char *const right)
{
    char  *head;
	    char  *tail;
	    size_t length;
	    char  *result;

	    if ((string == NULL) || (left == NULL) || (right == NULL))
	        return NULL;
	    length = strlen(left);
	    head   = strstr(string, left);
	    if (head == NULL)
	        return NULL;
	    head += length;
	    tail  = strstr(head, right);
	    if (tail == NULL)
	        return tail;
	    length = tail - head;
	    result = malloc(1 + length);
	    if (result == NULL)
	        return NULL;
	    result[length] = '\0';

	    memcpy(result, head, length);
	    return result;

}

/*
 * Name:	extractRowMatchingPattern
 * Input:	char *text, char *pattern
 * Output:	char * A line of text
 * Description:	It returns a line within a text including a pattern
 */

char * extractRowMatchingPattern(char *text, char *pattern)
{
	char * line = (char *)malloc(1024);

	if (line == NULL)
	{
		printf("Malloc failure: extractRowMatchingPattern\n");
		exit(-1);
	}

	line = strstr(text, pattern);
	if (line == NULL)
	{
		printf("Fatal error: extractRowMatchingPattern: pattern %s was not found in predictor output file\n", pattern);
		exit(-1);
	}
	line = line + strlen(pattern) + 1;

	return line;
}

/*
 * Name:		extractRowN
 * Input:		char *text, int row
 * Output:		char * A line
 * Description:	It extracts from a text the row in position "row"
 */

char * extractRowN(char *text, int row)
{
	int len = strlen(text);
	int iText, iLine = 0;
	char * line = (char *)malloc(BIG_LINE);

	if (line == NULL)
	{
		printf("Malloc failure: edxtractRowN\n");
		exit(-1);
	}

	int countRow = 0;

	iText = 0;

	strcpy(line, "");

	while ( countRow < row && iText < len)
	{
		iLine = 0;
		while(text[iText] != '\n' &&
				iText < strlen(text))
		{
			line[iLine++] = text[iText++];
		}
		countRow++;
		iText++;
	}

	if (row > countRow) return "stop";
	line[iLine] = '\0';

	if (line == NULL)
	{
		printf("Fatal error: extractRowN: returned string cannot be NULL\n");
		exit(-1);
	}
	return line;
}


/*
 * Name:	extractWord
 * Input:	char * line, int pos
 * Output: char * A word
 * Description: It returns the word of position "pos" in a line of text
 */
char * extractWord(char * line, int pos)
{

	char *word = (char *)malloc(64);

	if (word == NULL)
	{
		printf("Malloc failure: extractWord\n");
		exit(-1);
	}

	int lineIndex = 0;
	int wordIndex = 0;
	int len = strlen(line);
	int countwords = 0;

	while (lineIndex <= len)
	{
		if (line[lineIndex] != '\t') word[wordIndex++] = line[lineIndex++];
		else
		{
			countwords++;
			if (countwords == pos)
			{
				word[wordIndex] = '\0';
				break;
			}
			wordIndex = 0;
			lineIndex++;
		}
	}

	if (word == NULL)
	{
		printf("Fatal error: extracWord: returned string is NULL\n");
		exit(-1);
	}
	return word;
}


/*
 * Name:		replace
 * Input:		char * text, char *newLine
 * Output:		char * The text including the replaced value
 * Description:	It replaces the value of the computed nodes in a text representing the lua file content
 */
char * replace(char * text, char *newLine)
{

	int lineCount;
	char line[BIG_LINE];
	char *newText = (char *)malloc(BIG_TEXT);

	if (newText == NULL)
	{
		printf("Malloc failure: replace\n");
		exit(-1);
	}

	if (newLine == NULL)
	{
		printf("Fatal error: replace: newline cannot be null\n");
		exit(-1);
	}
	lineCount = 1;

	strcpy(newText, "");
	strcpy(line, extractRowN(text, lineCount));
	while ( strcmp(line, "stop") != 0)
	{
		if (strstr(line, "Nodes") != NULL) strcat(newText, newLine);
		else strcat(newText, line);
		strcat(newText, "\n");
		lineCount++;
		strcpy(line, extractRowN(text, lineCount));
	}

	return newText;

}


