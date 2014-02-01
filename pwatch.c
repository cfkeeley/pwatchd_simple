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
 
/* pwatch 0.4 BETA - crizza 12/08/04 
 *
 * TODO
 * 	*) udp listener
 *
 * 	*) implement rest of config file options (see pwatch.conf)
 *
 * 	*) pass in active_socket array to report_status_and_bail()
 * 	   so we can make sure all sockets are closed on exit from anywhere
 *
 * ------------------------------------------------------------------------- 
 * changelog: 
 * -------------------------------------------------------------------------
 *
 * 	26/08/04 ver 0.2 - changed storage for various arrays to heap 
 * 		   	   moved all includes to respective header files 
 * 		   	   tidied up some code.  
 *
 * 	27/08/04 ver 0.3 - added function to check for duplicate connections
 * 			   using the existing active ruleset rather than a file
 * 			   or list of hosts.
 *
 * 	03/09/04 ver 0.4 - collected option variables into a global struct
 * 			   added token parsing routine: get_tokens(struct opt *) 
 * 			   found in conf.c.
 * 			   added routine to check for interface on host. 
 *
 *	17/09/04 ver 0.4 - moved tcp init function to seperate file 	
 *			   added functionality to include PARSE_ENTIRE_RULESET
 *			   config option. Tidied up some code.
 *
 * --------------------------------------------------------------------------
 */

#define _BSD_SOURCE
#include "pwatch.h"

void exit();
void sig_handle(int sig);

/* the global conditional 
 * false if a signal has been caught */
int safe = 1;

void sig_handle(int sig)
{
	int save_errno = errno;
	safe = 0;
	errno = save_errno;
}

int main(int argc, char **argv) 
{
	struct sigaction sa;
	struct sockaddr_in remote, sock_info;
	struct opt opts;
	size_t size = 0;
	extern int safe;
	int i = 0, fdselect = 0, incoming = 0, portnum = 0;
	int daemon_mode = 0, connect = 0; 
	int *tcp_sockfd_arr, *udp_sockfd_arr; /* temporary socket arrays */
	socklen_t addrlen;
	mode_t mask = 0;
	fd_set pwatch_select_fdset;
	pid_t pid = 0, sid = 0;
	uid_t user_id;
	char client_buffer[16];
	size_t bufsize = sizeof(client_buffer);

	memset(&remote, 0, sizeof(struct sockaddr_in));
	memset(client_buffer, 0, 4);
	memset(&pwatch_select_fdset, 0, sizeof(fd_set));
	memset(&sa, 0, sizeof(struct sigaction));
	memset(&opts, 0, sizeof(struct opt));

	addrlen = sizeof(remote);
	size = sizeof(remote);

	opts.portstring = (char *)pwatch_malloc(opts.portstring, SOCK_LIMIT);
	opts.tcp_ports_ptr = (int *)pwatch_malloc(opts.tcp_ports_ptr, SOCK_LIMIT);
	opts.failed_ports = (int *)pwatch_malloc(opts.failed_ports, SOCK_LIMIT);
	
	mask |= ( S_IWGRP | S_IWOTH ); /* 022 umask for child */
	
	sa.sa_handler = sig_handle;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
	sigaction(SIGSYS, &sa, NULL);
	sigaction(SIGILL, &sa, NULL);

	FD_ZERO(&pwatch_select_fdset);
	
	(void)fprintf(stderr, "------------------------------\n");
	(void)fprintf(stderr, "%s\n", VERSION);
	(void)fprintf(stderr, "------------------------------\n");

	user_id = getuid();
	/* only root should have access to the pf device and also ports <= 1024 */
	if(user_id) {
		(void)fprintf(stderr, "=> ALERT: you need to be root to run this program\n");
		(void)logmsg("err: userid:%d does not have required privelidge\n", user_id);
		exit(1);
	}

	(void)fprintf(stderr, "=> parsing config file: ");

	/* parse the config file */
	if((get_tokens(&opts)))
			(void)fprintf(stderr, "- ok");

	(void)fprintf(stderr, "\n");
	(void)fprintf(stderr, "=> using interface: %s\n", opts.ifname);

	/* a tmp udp array will also be allocated. Both TCP and UDP 
	 * arrays will be copied into opts.socket_union 
	 * this union will then be used in select() and the tmp arrays freed */
	if(opts.flags & LISTEN_TCP) {
		tcp_sockfd_arr = (int *)malloc(sizeof(opts.num_tcp_ports));
		(void)tcp_init(&opts, tcp_sockfd_arr);
	}

	/*
	if(opts.flags & LISTEN_UDP)
		(void)udp_init(&opts, active_sockFD_array);
		*/

	(void)fprintf(stderr, "=> ok, entering daemon mode\n");
	(void)logmsg("entering daemon mode");
	
	/* prepare for daemon mode, fork for child */
	if((pid = fork()) > 0) 
		_exit(0); 
	else {
		daemon_mode |= FORK_SUCCESS;
		umask(mask);
	}

	if((sid = setsid()) > 0) 
		daemon_mode |= SID_SUCCESS;

	if(daemon_mode & (FORK_SUCCESS | SID_SUCCESS)) {

		for(i = 0; i < 3; i++)
			(void)close(i); /* close std fd's */

		while(safe) { /* polling loop - bit lame, may use threads instead */
			for(i = 0; i < opts.num_sockets_bound; i++)
				FD_SET(tcp_sockfd_arr[i], &pwatch_select_fdset);

			fdselect = select(SOCK_LIMIT+1, &pwatch_select_fdset, NULL, NULL, (struct timeval *)NULL);

			if(fdselect < 0) 
				(void)logmsg("select returned %d", fdselect);
			else if(fdselect == 0) 
				; /* no timeout */
			else if(fdselect > 0)  
				for(i = 0; i < opts.num_sockets_bound && safe; i++) 
					if(FD_ISSET(tcp_sockfd_arr[i], &pwatch_select_fdset) && safe) {
						incoming = accept(tcp_sockfd_arr[i], (struct sockaddr *)&remote, &addrlen);
						
						if(incoming < 0) break; else connect = 1;
							
						(void)getsockname(tcp_sockfd_arr[i], (struct sockaddr *)&sock_info, &addrlen);
						portnum = ntohs(sock_info.sin_port);	

						(void)inet_ntop(AF_INET, &remote.sin_addr.s_addr, client_buffer, bufsize);

						(void)logmsg("connection from %s on port %d", client_buffer, portnum);
				
						if((close(incoming)) < 0) {
							(void)logmsg("err: failed to close socket %d", incoming);
							(void)report_status_and_bail(BAIL_OUT, errno);
						}
						break;
					}
			if(connect && safe) /* maybe this should persist with a 'while loop' 
				     * in case pf_block_probe() fails? 
				     * could cause some real problems 
				     * if pf_block_probe() fails to terminate process 
				     * due to a serious error! */
				if(pf_block_probe(client_buffer, portnum, &opts)) {
					connect = 0;
					memset(client_buffer, 0, bufsize);
				}
					
		} /* !_polling loop */
		
		for(i = 0; i < opts.num_sockets_bound; i++)
			if((shutdown(tcp_sockfd_arr[i], SHUT_RDWR)) < 0)
				(void)logmsg("err: shutdown failure on socket descriptor:%d", tcp_sockfd_arr[i]);
		
		(void)report_status_and_bail(BAIL_OUT, errno);
		
	} else { 
		(void)fprintf(stderr, "=> failed to enter daemon mode\n");
		(void)fprintf(stderr, "=> terminating process");
		(void)logmsg("failed to enter daemon mode");
		(void)report_status_and_bail(BAIL_OUT, errno);
	}
return(0);
}
