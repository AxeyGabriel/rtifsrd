/*-
 * Realtime Interface Statistics Receiver Daemon
 * rtifsrd
 * 
 * Written by Axey Gabriel Muller Endres
 * 10 Aug 2021
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <syslog.h>
#include <zmq.h>

int main(int argc, char **argv)
{
	int rc;
	void *context = zmq_ctx_new();
	void *senders = zmq_socket(context, ZMQ_XSUB);
	zmq_bind(senders, "tcp://*:5555");
	
	void *forward = zmq_socket(context, ZMQ_XPUB); 
	zmq_bind(forward, "tcp://*:5556");
	
	zmq_pollitem_t items[] = 
	{
		{ senders, 0, ZMQ_POLLIN, 0 }
	};
	
	while (1)
	{
		zmq_msg_t message;
		
		zmq_poll(items, 1, -1);
		if (items[0].revents & ZMQ_POLLIN)
		{
			printf("Message received!\n");
			while (1)
			{
				zmq_msg_init(&message);
				zmq_msg_recv(&message, senders, 0);
				int more = zmq_msg_more(&message);
				zmq_msg_send(&message, forward, more ? ZMQ_SNDMORE : 0);
				zmq_msg_close(&message);
				if (!more)
				{
					break;
				}
			}
		}
	}
	
	zmq_close(senders);
	zmq_close(forward);
	zmq_ctx_destroy(context);
	
	return 0;
}
