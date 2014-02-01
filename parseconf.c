/* Copyright (c) 1982, 1986, 1990, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "parseconf.h"
#include <errno.h>
#include <unistd.h>


static int add_delim(char *); 
static char *delim_check(char *, char *);

/* this is not the most 'economical' implementation for reading in config file options
 * however, it is the easiest! 
 * TODO: needs to be tested with a dodgy config file to check for bugs */
int get_conf_token(char *token, char *conbuf, size_t size)
{
	FILE *fp;
	char *ptr = NULL, buf[MAXBUFFER];
	int ret = 0;

	fp = fopen(CONF_FILE_PATH, "r");
	if(!fp) {
		(void)fprintf(stderr, "failed to open %s\n", CONF_FILE_PATH);
		perror("fopen");
		exit(1);
	}

	while((fgets(buf, MAXBUFFER, fp)) != NULL) {
		if(buf[0] == '#' || buf[0] == ' ') /* skip comments and leading whitespace */
			continue;
		if(strncmp(token, buf, strlen(token)) == 0) {
			ptr = strchr(buf, '"');
			ptr = delim_check(ptr, token);
			memcpy(conbuf, ptr, strlen(ptr));
			ret = 1;
		}
	}

	if(fp); fclose(fp);  

return ret;
}

/* check delim and value are present */
static char *delim_check(char *ptr, char *token)
{
	if(ptr == NULL) {
		(void)fprintf(stderr, "error\n=> token:%s - %s\n", token, NO_DELIM);
		exit(1);
	}  

	ptr++; /* move past initial delimeter '"' */
			
	if(*ptr == '"') {
		(void)fprintf(stderr, "error\n=> token:%s - %s\n", token, NO_VAL);
		exit(1);
	}  

	if(! add_delim(ptr)) {
		(void)fprintf(stderr, "error\n=> token:%s - %s\n", token, NO_DELIM);
		exit(1);
	}

return ptr;
}

/* replace final '"' with terminating '\0' */
static int add_delim(char *line)
{
	char *ptr;
	int ret = 1;
	ptr = strchr(line, '"');

	if(ptr == NULL) {
		ret = 0;
	} else 
		*ptr = '\0';
return ret;
}


/* seperate ports, convert to int then assign to ptr array 
 * return the total number of ports */
int parseports(char *ports, int *ports_ptr)
{
	int i = 0;
	char *tok = NULL;
	int *portptr, port_arr[SOCK_LIMIT], num_ports = 0;

	tok = strtok(ports, ",");
	while(tok != NULL) {
		ports_ptr[i] = atoi(tok);
		tok = strtok(NULL, ",");
		i++;
	}
	num_ports = i;

return num_ports;
}

