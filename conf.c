#include "conf.h"
#include "getipv4.h"
/* 
 * parse all the ops out of the config files and set their values in the opts struct 
 * do straight exit on errors. 
 *
 * it gets *ugly* in here :( thus, the extra spacing 
 *
 * TODO this monster may need to be re-written before it gets too big. 
 *
 */

static void check_portrange(int);
static void check_interface(char *);
void sync_opts(struct opt *);


int get_tokens(struct opt *opts)
{
	u_char tmp[1], conv; /* for binary config options */

	if(! (get_conf_token(token[IF_NAME], opts->ifname, sizeof(opts->ifname)))) {

		(void)fprintf(stderr, "[ error: '%s' not found! ]\n", token[IF_NAME]); 
		(void)logmsg("config err: interface '%s' not found!", token[IF_NAME]);
		exit(1);

	} else {

		(void)fprintf(stderr, "%s ", token[IF_NAME]);

		if(*opts->ifname == NULL) {

			(void)fprintf(stderr, "err: no interface specified!\n");
			(void)logmsg("config err: no interface specified");
			exit(1);

		/* check host for interface */
		} else if(! check_iface_exists(opts->ifname)) {

			(void)fprintf(stderr, "\n=> err: interface '%s' not found/active!\n", opts->ifname);
			(void)logmsg("err: interface:%s not found/active!", opts->ifname);
			exit(1);
		}
	}
	
	if(! (get_conf_token(token[TCP_PORT_LIST], opts->portstring, SOCK_LIMIT))) {

		(void)logmsg("err: '%s' not found! using default", token[TCP_PORT_LIST]);
		opts->flags & (~LISTEN_TCP); /* default: off*/

	} else 
		if(opts->portstring != NULL) {

			(void)fprintf(stderr, "%s ", token[TCP_PORT_LIST]);

			if(atoi(&opts->portstring[0]) == OFF) 
				opts->flags & (~LISTEN_TCP); /* default: off */
			else {
				opts->num_tcp_ports = parseports(opts->portstring, opts->tcp_ports_ptr);
				opts->flags |= LISTEN_TCP; 
				(void)free(opts->portstring);
				(void)check_portrange(opts->num_tcp_ports);
			}

		} 

	if(! (get_conf_token(token[REUSE_SOCK], tmp, sizeof(tmp)))) {

		(void)logmsg("config err: '%s' not found! using default", token[REUSE_SOCK]);
		opts->flags & (~REUSE_SKTS); /* default: off */

	} else {

		(void)fprintf(stderr, "%s ", token[REUSE_SOCK]);

		conv = atoi(tmp);

		if(conv == ON)
			opts->flags |= REUSE_SKTS;
	}

	if(! (get_conf_token(token[PARSE_ENTIRE_RULESET], tmp, sizeof(tmp)))) {

		(void)logmsg("config err: '%s' not found! using default", token[PARSE_ENTIRE_RULESET]);
		opts->flags & (~PARSE_ALL); /* default: off */

	} else {

		(void)fprintf(stderr, "%s ", token[PARSE_ENTIRE_RULESET]);

		conv = atoi(tmp);

		if(conv == ON)
			opts->flags |= PARSE_ALL;
	}

	if(! (get_conf_token(token[USE_ANCHOR], tmp, sizeof(tmp)))) {

		(void)logmsg("config err: '%s' not found! using default", token[USE_ANCHOR]);
		opts->flags & (~USE_ANC); /* default: off */

	} else {

		(void)fprintf(stderr, "%s ", token[USE_ANCHOR]);

		conv = atoi(tmp);

		if(conv == ON)
			opts->flags |= USE_ANC;
	}

return 1;
}


static void check_portrange(int num_ports)
{
	if(num_ports > SOCK_LIMIT) {
		(void)fprintf(stderr, "\n=> err: number of ports specified too large.\n");
		(void)fprintf(stderr, "=> refer to 'SOCK_LIMIT' in parseconf.h\n");
		exit(1);
	}
}

/* check options are in sync with each other */ 
void sync_opts(struct opt *opts)
{


}
