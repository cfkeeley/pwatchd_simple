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

#include "getipv4.h"
/*#include "pwatch_filter.h"*/

int get_interface_ipv4(int, char *, struct sockaddr *, char *);

/* get current ipv4 address for interface (ifname)  */
int get_interface_ipv4(int sock, char *ifname , struct sockaddr *ifaddr, char *buff)
{
	struct ifreq *ifr;
	struct ifreq ifrr;
	struct sockaddr_in sa;
	int ret = 1;

	ifr = &ifrr;
	ifrr.ifr_addr.sa_family = AF_INET;

	strlcpy(ifrr.ifr_name, ifname, sizeof(ifrr.ifr_name));

	if((ioctl(sock, SIOCGIFADDR, ifr)) < 0)
		ret = 0;

	strlcpy(buff, inet_ntoa(inaddrr(ifr_addr.sa_data)), sizeof(struct sockaddr));

return ret;
}

int check_iface_exists(char *name)
{
	struct ifreq *ifr;
	struct ifreq ifrr;
	int ret = 1;
	int tmp_sockfd = get_dgram_socket();

	ifr = &ifrr;
	ifrr.ifr_addr.sa_family = AF_INET;
	strlcpy(ifrr.ifr_name, name, sizeof(ifrr.ifr_name));

	if((ioctl(tmp_sockfd, SIOCGIFADDR, ifr)) < 0) 
		ret = 0;

	close(tmp_sockfd);

return ret;
}
