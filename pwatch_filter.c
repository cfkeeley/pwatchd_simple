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
 *
 * ----------------------------------------------------------------------------
 * pwatch_filter.c - routines to add and check pf rules 
 *
 */

#include "pwatch_filter.h"

/* keep compiler quiet */
int inet_pton(int, const char *, void *);
int close(int);
/*  */

inline static int test_rule(char *, int *, int, int, u_char);


/*  socket for ioctl SIOCGIFADDR to get current interface ipv4 address */
int get_dgram_socket() 
{
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0)
		(void)logmsg("failed to open dgram socket");
return sockfd;
}


int pf_block_probe(char *incoming_ip, int portnum, struct opt *opts) 
{
	struct pfioc_rule pr;
	struct pfioc_pooladdr paddr;
	struct sockaddr ifr;
	char curr_ipv4[16];
	char *ifname = opts->ifname;
	short test_result = 0;
	int pf_dev_fd, ret = 1, sockfd;
	static int num_rules_added = 0; /* the rule number to add the next rule after */

	memset(&pr, 0, sizeof(struct pfioc_rule));
	memset(&pr.rule, 0, sizeof(struct pf_rule));
	memset(&paddr, 0, sizeof(struct pfioc_pooladdr));

	if((sockfd = get_dgram_socket()) < 0) {
		(void)logmsg("get_dgram_socket() no socket for getipv4()");
		(void)report_status_and_bail(BAIL_OUT, errno);
	}
		
	if(! (get_interface_ipv4(sockfd, ifname, &ifr, curr_ipv4))) {
		(void)logmsg("Unable to obtain ipv4 for interface %s\n", ifname);
		(void)close(sockfd);
		(void)report_status_and_bail(BAIL_OUT, errno);
	}
	else 
		(void)close(sockfd); 

	/* init rule structure */
	pr.rule.action = PF_DROP;	
	pr.rule.quick = 1;
	pr.rule.log = 1;
	pr.rule.af = AF_INET;
	pr.rule.proto = IPPROTO_TCP;
	pr.rule.src.addr.type = PF_ADDR_ADDRMASK;
	memset(&pr.rule.src.addr.v.a.mask.v4, 255, 4); /* match only this host:port */
	inet_pton(AF_INET, incoming_ip, &pr.rule.src.addr.v.a.addr.v4);
	pr.rule.src.addr.type = PF_ADDR_ADDRMASK;
	memset(&pr.rule.dst.addr.v.a.mask.v4, 255, 4);
	inet_pton(AF_INET, curr_ipv4, &pr.rule.dst.addr.v.a.addr.v4); 
	pr.rule.dst.port_op = PF_OP_EQ;
	pr.rule.keep_state = 0;
	pr.rule.flags = TH_SYN;
	pr.rule.flagset = (TH_SYN | TH_ACK | TH_FIN | TH_RST);
	pr.rule.dst.port[0] = htons(portnum);
		
	pf_dev_fd = open("/dev/pf", O_RDWR); 
	
	if(pf_dev_fd < 0) {
		(void)logmsg("failed to open /dev/pf, *not* adding rule\n");
		(void)logmsg("connection attempt from %s on port %d _NOT_ BLOCKED\n", incoming_ip, portnum);
		(void)report_status_and_bail(DO_NOT_BAIL, errno);
		(void)close(pf_dev_fd);
		ret = 0;
	}

	/* check the rule doesn't already exist in rules added so far */
	if(ret)
		test_result = test_rule(incoming_ip, &pf_dev_fd, portnum, num_rules_added, opts->flags);

	if(test_result && ret) {

		if((ioctl(pf_dev_fd, DIOCBEGINADDRS, &paddr)) < 0) 
			logmsg("err: ioctl DIOCBEGINADDRS");

		pr.action = PF_CHANGE_GET_TICKET;
		pr.pool_ticket = paddr.ticket;
	
		if((ioctl(pf_dev_fd, DIOCCHANGERULE, &pr)) < 0)
			logmsg("err: ioctl DIOCCHANGERULE");

		pr.action = PF_CHANGE_ADD_AFTER;
		pr.nr = num_rules_added;
		
		if((ioctl(pf_dev_fd, DIOCCHANGERULE, &pr)) < 0)
			logmsg("err: ioctl DIOCCHANGERULE");

		num_rules_added++; /* static */

		(void)logmsg("added blocking filter rule for remote host %s on port %d", incoming_ip, portnum); 
		
	} else if(test_result == 0) 
		/* some moron may attempt to fill up the log file! 
	         * this *may* become an issue during testing. 
		 * Here, a zero tolerance response could be taken 
		 * for repeat offenders; for now, just ignore and log */ 
		(void)logmsg("rule already exists, ignoring");
	else {
		(void)logmsg("connection %s on port %d", incoming_ip, portnum);
		(void)logmsg("ioctl: error searching current ruleset - *no* rule added");
	}

	(void)close(pf_dev_fd);
return ret;
}

/*
 * check we are not adding duplicate entries to the ruleset 
 * return 1 on unique 0 for duplicate -1 for error 
 * since we added after rule:0 we only parse the subesquent rules added 
 * by pwatch unless we are parsing the *entire* active ruleset 
 */
inline static int test_rule(char *incoming_ip, int *pfdev, int portnum, int num_rules_added, u_char flags)
{
	char ret = 1;
	register int i;
	int port_from_rset;
	struct pfioc_rule rset_chk;
	char *ip_from_rset = NULL;

	memset(&rset_chk, 0, sizeof(rset_chk));

	if((ioctl(*pfdev, DIOCGETRULES, &rset_chk)) < 0) {
		(void)logmsg("err: ioctl DIOCGETRULES in test_rule()");
		ret = PWATCH_ERR;
	}

	if(flags & PARSE_ALL) /* parse entire ruleset */
		num_rules_added = rset_chk.nr; /* total number of rules in 
						* active set */

	if(ret) /* true if we have ticket */ 
		for(i = 0; i < num_rules_added; i++) {
			
			rset_chk.nr = i;

			if((ioctl(*pfdev, DIOCGETRULE, &rset_chk)) < 0) {
				(void)logmsg("err: ioctl DIOCGETRULE in test_rule()");
				ret = PWATCH_ERR;
			}

			ip_from_rset = inet_ntoa(rset_chk.rule.src.addr.v.a.addr.v4);
			port_from_rset = htons(rset_chk.rule.dst.port[0]);

			if((strncmp(incoming_ip, ip_from_rset, strlen(incoming_ip)) == 0) \
				&& (portnum == port_from_rset)) {
					ret = 0;	
					break; /* ok, we got a match - return */
			}
		}	
return ret;
}

