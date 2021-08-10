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
#include <fcntl.h>
#include <signal.h>

int signal_fd;

void sighandler(int signal)
{
	int rc = write(signal_fd, " ", 1);
	if (rc != 1)
	{
		syslog(LOG_ERR, "Signal handler: Error writing to pipe");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	int rc;
	int pipefds[2];

	openlog("rtifsrd", LOG_PID | LOG_NDELAY, LOG_LOCAL0);
	syslog(LOG_INFO, "System started, initializing...");
	
	rc = pipe(pipefds);
	if (rc != 0)
	{
		syslog(LOG_ERR, "Error creating pipe: %m");
		return 1;
	}
	
	int i;
	for (i = 0; i < 2; i++)
	{
		int flags = fcntl(pipefds[i], F_GETFL, 0);
		if (flags < 0)
		{
			syslog(LOG_ERR, "Error: fcntl: %m");
			return 1;
		}
		rc = fcntl(pipefds[i], F_SETFL, flags | O_NONBLOCK);
		if (rc != 0)
		{
			syslog(LOG_ERR, "Error: fcntl: %m");
			return 1;
		}
	}
	
	signal_fd = pipefds[1];
	
	void *context = zmq_ctx_new();
	if (context == NULL)
	{
		syslog(LOG_ERR, "zmq_ctx_new error: %s", zmq_strerror(errno));
		return 1;
	}
	void *senders = zmq_socket(context, ZMQ_XSUB);
	if (senders == NULL)
	{
		syslog(LOG_ERR, "zmq_socket error: %s", zmq_strerror(errno));
		return 1;
	}
	
	const char *filter = "";
	rc = zmq_setsockopt(senders, ZMQ_SUBSCRIBE, &filter, sizeof(filter));
	
	rc = zmq_bind(senders, "tcp://*:5555");
	if (rc == -1)
	{
		syslog(LOG_ERR, "Error: zmq_bind: %s", zmq_strerror(errno));
		return 1;
	}
	
	void *forward = zmq_socket(context, ZMQ_XPUB); 
	rc = zmq_bind(forward, "tcp://*:5556");
	if (rc == -1)
	{
		syslog(LOG_ERR, "Error: zmq_bind: %s", zmq_strerror(errno));
		return 1;
	}
	
	zmq_pollitem_t items[] = 
	{
		{ senders, 0, ZMQ_POLLIN, 0 },
		{ 0, pipefds[0], ZMQ_POLLIN, 0}
	};
	
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	
	while (1)
	{
		zmq_msg_t message;
		
		zmq_poll(items, 2, -1);
		if (items[0].revents & ZMQ_POLLIN)
		{
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
		if (items[1].revents & ZMQ_POLLIN)
		{
			char buffer[1];
			read(pipefds[0], buffer, 1);
			syslog(LOG_NOTICE, "Interrupt received, stopping system.");
			break;
		}
	}
	
	zmq_close(senders);
	zmq_close(forward);
	zmq_ctx_destroy(context);
	
	return 0;
}
