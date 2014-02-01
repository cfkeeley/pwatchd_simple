#ifndef HDR_CONF
#define HDR_CONF

#include <stdio.h>
#include "utils.h"
#include "parseconf.h" /* struct opts */

int get_tokens(struct opt *);
enum status { OFF, ON };

static u_char *token[] = { "TCP_PORT_LIST", "UDP_PORT_LIST", "INTERFACE", "REUSE_SOCKETS", \
	 "PARSE_ENTIRE_RULESET", "USE_ANCHOR", "ANCHOR_NAME", "PKT_SCAN" };

#endif /* !_HDR_CONF conf.h */
