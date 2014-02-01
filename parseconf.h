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

#ifndef HDR_PARSECONF
#define HDR_PARSECONF

#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <sys/types.h>
#include "utils.h" /* BAIL_OUT */

#define MAXBUFFER 1024 /* max read bytes from file in one go */
#define SOCK_LIMIT 250 /* maximum sockets both tcp and udp */
#define MAX_IFNAME_LEN 5
#define CONF_FILE_PATH "pwatch.conf" /* currently in the same dir but
				      *	should be changed to /etc/pwatch
				      *	for working release 
			              */
#define NO_DELIM "missing '\"' delimeter"
#define NO_VAL "has no value"

/* config flags - some of these are _not_ yet implemented */
#define PARSE_ALL 0x01 /* parse entire ruleset */
#define REUSE_SKTS 0x02 /* reuse only pwatch sockets */
#define USE_ANC 0x04 /* use a ruleset anchor */
#define LISTEN_TCP 0x08 /* monitor tcp */
#define LISTEN_UDP 0x10 /* monitor udp */
#define SCAN_PKT 0x20 /* scan packets */

/* constants for index into token[]   
 * e.g token[TCP_PORT_LIST] == "TCP_PORT_LIST" */
enum token_opts { TCP_PORT_LIST, UDP_PORT_LIST, IF_NAME, REUSE_SOCK, PARSE_ENTIRE_RULESET, USE_ANCHOR, PKT_SCAN };

int get_conf_token(char *, char *, size_t);
int parse_ports(char *, int *);

/* this holds all of our variable options */
struct opt {
	char *portstring;
	unsigned char ifname[MAX_IFNAME_LEN];
	int *tcp_ports_ptr;
	int *udp_ports_ptr;
	int *failed_ports;
	int socket_union[SOCK_LIMIT]; /* union of tcp and udp sockets */
	unsigned int num_tcp_ports;
	unsigned int num_udp_ports;
	unsigned int num_failed;
	unsigned int num_sockets_bound;
	unsigned char flags; /* options from config file */
};

#endif /* parseconf.h */
