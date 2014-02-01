#include "tcp_init.h"

void tcp_init(struct opt *opts, int *sockfd_array)  
{
	register int i;

	if(opts->flags & LISTEN_TCP) {

		(void)fprintf(stderr, "=> binding tcp sockets\n");

		for(i = 0; i < opts->num_tcp_ports; i++) {

			if((sockfd_array[opts->num_sockets_bound] = get_stream_sockfd(opts->flags)) < 0 ) {

				(void)fprintf(stderr, "error: failed to get socket\n");
				(void)logmsg("get_stream_sockfd() failure");
				exit(1);
				continue;	

			}

			if(! (bindsockfd(sockfd_array[opts->num_sockets_bound], opts->tcp_ports_ptr[i]))) {
				
				opts->failed_ports[opts->num_failed] = opts->tcp_ports_ptr[i];
				opts->num_failed++;

			} else 
				opts->num_sockets_bound++;
		}

		(void)free(opts->tcp_ports_ptr);
		opts->tcp_ports_ptr = NULL;

		if(opts->num_sockets_bound == 0) {

			(void)fprintf(stderr, "=> error: failed to bind any sockets, quitting\n");
			(void)logmsg("err: failed to bind any sockets, quitting");
			exit(1);

		} else {

			(void)fprintf(stderr, "=> tcp sockets bound: %d/%d\n", opts->num_sockets_bound, opts->num_tcp_ports);
			(void)logmsg("bound %d/%d sockets", opts->num_sockets_bound, opts->num_tcp_ports);

			if(opts->num_failed) {
				(void)fprintf(stderr, "=> bind failed on port:");

				for(i = 0; i < opts->num_failed; i++)
					(void)fprintf(stderr, " %d", opts->failed_ports[i]);

				(void)fprintf(stderr, "\n");
			}
		}
		
		(void)free(opts->failed_ports);
		opts->failed_ports = NULL;
	}
}
