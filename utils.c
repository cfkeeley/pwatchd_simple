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

#include "utils.h"
#include "pwatch_filter.h"

/* allocate memory and check
 * return ptr to new memory on success or
 * exit on failure */
void *pwatch_malloc(void *ptr, size_t size)
{
	ptr = (void *)malloc(size);
	if(!ptr) {
		(void)fprintf(stderr, "memory allocation error");
		perror("pwatch_malloc");
		exit(1);	
	}
return ptr;
}

int get_stream_sockfd(int opt_flags) 
{
	int sockfd, ret;
	int one = 1;

	if(opt_flags & SCAN_PKT) {
		if((sockfd = socket(AF_INET, SOCK_RAW, 0)) < 0)
			(void)logmsg("error: get_stream_sockfd() descriptor = %d", sockfd);
	} else {
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			(void)logmsg("error: get_stream_sockfd() descriptor = %d", sockfd);
	}

	if(opt_flags & REUSE_SKTS)
		ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)); 
	if(ret < 0)
		(void)logmsg("setsockopt:SO_REUSEADDR socket level failure");
return sockfd;
}

/* do the binding */
int bindsockfd(int sockfd, int port) 
{
	struct sockaddr_in local;
	int ret = 1;

	if(port == 0)
		return 0;
	
	memset(&local, 0, sizeof(struct sockaddr_in));
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = INADDR_ANY;

	if((bind(sockfd, (struct sockaddr *)&local, sizeof(local))) != 0) {
		(void)close(sockfd);
		ret = 0;
	} else 	
		listen(sockfd, MAX_QUEUED_CLIENTS);

return ret;
}

void report_status_and_bail(int bail_out, int err)
{
	openlog("pwatchd:", LOG_PID | LOG_PERROR, LOG_DAEMON);  
	syslog(LOG_EMERG, "syslog broadcast: %m", err);

	if(bail_out) { 
		(void)logmsg("pwatchd shutting down");
		exit(1);
	}
}

