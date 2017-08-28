//
//  gateway.c
//
//  Created by Martin Lane-Smith on 7/2/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//
// Starts all processes

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <termios.h>

#include <string>
#include <vector>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

#include "software_profile.h"
#include "gateway.h"

#include "ps.h"
#include "ps_message.h"
#include "ps_common.h"
#include "syslog/ps_syslog_linux.hpp"
#include "common/ps_root_class.hpp"
#include "pubsub/ps_pubsub_class.hpp"
#include "serial/linux_serial/ps_serial_linux.hpp"
#include "serial/socket/ps_socket_server.hpp"
#include "packet/serial_packet/ps_packet_serial_linux.hpp"
#include "packet/xbee_packet/ps_packet_xbee_linux.hpp"
#include "packet/xbee_packet/ps_packet_xbee_class.hpp"
#include "transport/ps_transport_linux.hpp"
#include "network/ps_network.hpp"
#include "patch/patch_class.hpp"
#include "responder/responder.hpp"

#include "BBB/common.h"
#include "BBB/uart.h"
#include "BBB/gpio.h"
#include "main_debug.h"

void KillAllOthers(std::string name);

FILE *mainDebugFile;

std::vector<patch_class *> patches;

void SIGHUPhandler(int sig);
int SIGHUPflag = 0;
void fatal_error_signal(int sig);
void SIGPIPE_signal (int sig){}

int main(int argc, const char * argv[]) {

	std::string initFail = "";

	//start the log
	the_logger();

	mainDebugFile = fopen_logfile("gateway");

	DEBUGPRINT("Gateway starting");

	//kill any others
	KillAllOthers(PROCESS_NAME);

	sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

	const struct sigaction sa {SIGPIPE_signal, 0, 0};
	sigaction(SIGPIPE, &sa, NULL);

	DEBUGPRINT("main() loading cape-universal...");

	//load universal dtos
	if (!load_device_tree("cape-universal")) {
		//error enabling pins
		ERRORPRINT("*** Load 'cape-universal' fail");
//		initFail = "dta1";
	} else {
		DEBUGPRINT("...loaded OK");
	}

	DEBUGPRINT("main() loading cape-univ-hdmi...");
	if (!load_device_tree("cape-univ-hdmi")) {
		//error enabling pins
		ERRORPRINT("*** Load 'cape-univ-hdmi' fail");
//		initFail = "dta2";
	} else {
		DEBUGPRINT("...loaded OK");
	}

	//start plumbing
	DEBUGPRINT("Registry init");
	//initialize the registry
	ps_register_event_names(eventNames, EVENT_COUNT);
	ps_register_condition_names(conditionNames, CONDITIONS_COUNT);
	ps_register_topic_names(psTopicNames, PS_TOPIC_COUNT);

	DEBUGPRINT("start gateway socket server");
	//socket server
	ps_socket_server *g_socket_server = new ps_socket_server(
			GATEWAY_LISTEN_PORT, 0);
	//socket packet layer
	ps_packet_serial_linux *g_socket_pkt = new ps_packet_serial_linux("gateway_pkt", g_socket_server);
	//socket transport layer
	ps_transport_linux *g_socket_transport = new ps_transport_linux("gateway_tran", g_socket_pkt);
//	g_socket_transport->transport_source = SRC_IOSAPP;

    int i;
    for (i=0; i<SRC_COUNT; i++)
    {
    	g_socket_transport->source_filter[i] = false;
    }
    g_socket_transport->source_filter[SOURCE] = true;

	//add to pubsub network
	the_network().add_transport_to_network(g_socket_transport);

    the_broker().subscribe_to_topic(RESPONSE_TOPIC ,g_socket_transport);
    the_broker().subscribe_to_topic(SYS_REPORT_TOPIC ,g_socket_transport);

    the_broker().subscribe_to_packet(SYSLOG_PACKET ,g_socket_transport);
    the_broker().subscribe_to_packet(REGISTRY_UPDATE_PACKET ,g_socket_transport);
    the_broker().subscribe_to_packet(REGISTRY_SYNC_PACKET ,g_socket_transport);
    the_broker().subscribe_to_packet(CONDITIONS_PACKET ,g_socket_transport);


	//Robots
	DEBUGPRINT("init xbee module");

#define OUTPUT 1
#define INPUT 0
	gpio_export(XBEE_DTR_GPIO);
	gpio_set_direction(XBEE_DTR_GPIO, OUTPUT);	//output
	gpio_set_value(XBEE_DTR_GPIO, 0);		//active low

	gpio_export(XBEE_RTS_GPIO);
	gpio_set_direction(XBEE_RTS_GPIO, OUTPUT);	//output
	gpio_set_value(XBEE_RTS_GPIO, 1);

	gpio_export(XBEE_CTS_GPIO);
	gpio_set_direction(XBEE_CTS_GPIO, INPUT);	//input

	gpio_export(XBEE_STATUS_GPIO);
	gpio_set_direction(XBEE_STATUS_GPIO, INPUT);	//input

	gpio_export(XBEE_ASSOCIATE_GPIO);
	gpio_set_direction(XBEE_ASSOCIATE_GPIO, INPUT);	//input

	gpio_export(XBEE_RESET_GPIO);
	gpio_set_direction(XBEE_RESET_GPIO, OUTPUT);	//output
	gpio_set_value(XBEE_RESET_GPIO, 1);

	usleep(100000);									//reset the XBee

	gpio_set_value(XBEE_RESET_GPIO, 0);

	usleep(500000);

	//XBee serial port
	uart_setup(XBEE_TX_PIN, XBEE_RX_PIN);
	ps_serial_linux *xbee_serial = new ps_serial_linux("xbee_serial",
			XBEE_UART_DEVICE, XBEE_UART_BAUDRATE);
	//XBee module
	ps_packet_xbee_linux *xbee_module = new ps_packet_xbee_linux(xbee_serial);

	std::ifstream file("robot_list.txt");
	if (file) {
		int xbee, port;
		char name[20];

		while (1) {
			file >> xbee;
			if (xbee < 0 || file.eof())
				break;

			file >> port;
			file.get(name, 20);
			char *name_start = name;
			while (*name_start == ' ' && strlen(name_start) > 1) name_start++;

			DEBUGPRINT("robot: %s (xbee=%i, port=%i)", name_start, xbee, port);

			//patch class
			patch_class *patch = new patch_class(xbee_module, name_start, xbee, port);

			patches.push_back(patch);
		}
	}
	else
	{
		ERRORPRINT("Unable to open robot_list.txt");
	}

	ResponderInit();

	DEBUGPRINT("Init complete");

	if (getppid() == 1) {
		//child of init/systemd
		if (signal(SIGHUP, SIGHUPhandler) == SIG_ERR) {
			ERRORPRINT("SIGHUP err: %s", strerror(errno));
		} else {
			DEBUGPRINT("SIGHUP handler set");
		}
		//close stdio
		fclose(stdout);
		fclose(stderr);
		stdout = fopen("/dev/null", "w");
		stderr = fopen("/dev/null", "w");
	} else {
		signal(SIGILL, fatal_error_signal);
		signal(SIGABRT, fatal_error_signal);
		signal(SIGIOT, fatal_error_signal);
		signal(SIGBUS, fatal_error_signal);
		signal(SIGFPE, fatal_error_signal);
		signal(SIGSEGV, fatal_error_signal);
		signal(SIGTERM, fatal_error_signal);
		signal(SIGCHLD, fatal_error_signal);
		signal(SIGSYS, fatal_error_signal);
		signal(SIGCHLD, fatal_error_signal);
	}

	while (1) {
		//send pings;
		char buf[100];

		snprintf(buf, 100, "%s-%i", SOURCE_NAME, GATEWAY_LISTEN_PORT);
		ServerPing(buf, strlen(buf), PING_PORT_NUMBER);
//		DEBUGPRINT(buf);

		for (auto p = patches.begin(); p < patches.end(); p++) {
			patch_class *pc = *p;
			snprintf(buf, 100, "%s-%i", pc->name.c_str(),
					pc->socket_listen_port);

			if (pc->is_online())
			{
				ServerPing(buf, strlen(buf), PING_PORT_NUMBER);
//				DEBUGPRINT(buf);
			}
		}
		sleep(5);
	}

	return 0;
}
//SIGHUP
void SIGHUPhandler(int sig) {
	SIGHUPflag = 1;
	ERRORPRINT("SIGHUP signal");
}

//other signals
volatile sig_atomic_t fatal_error_in_progress = 0;
void fatal_error_signal(int sig) {
	/* Since this handler is established for more than one kind of signal, it might still get invoked recursively by delivery of some other kind of signal. Use a static variable to keep track of that. */
	if (fatal_error_in_progress)
		raise(sig);
	fatal_error_in_progress = 1;

	ERRORPRINT("Signal %i raised", sig);
	sleep(1);	//let there be printing

	/* Now re-raise the signal. We reactivate the signal�s default handling, which is to terminate the process. We could just call exit or abort,
	 but re-raising the signal sets the return status
	 from the process correctly. */
	signal(sig, SIG_DFL);
	raise(sig);
}

//helper functions to find any existing processes of a given name

/* checks if the string is purely an integer
 * we can do it with `strtol' also
 */
int check_if_number(char *str) {
	int i;
	for (i = 0; str[i] != '\0'; i++) {
		if (!isdigit(str[i])) {
			return 0;
		}
	}
	return 1;
}

#define MAX_BUF 1024
#define PID_LIST_BLOCK 32

//returns a list of up to 32 pids of processes matching the provided name
int *pidof(std::string pname) {
	DIR *dirp;
	FILE *fp;
	struct dirent *entry;
	int *pidlist, pidlist_index = 0, pidlist_realloc_count = 1;
	char path[MAX_BUF], read_buf[MAX_BUF];

	dirp = opendir("/proc/");
	if (dirp == NULL) {
		perror("Fail");
		return NULL;
	}

	pidlist = (int*) malloc(sizeof(int) * PID_LIST_BLOCK);
	if (pidlist == NULL) {
		return NULL;
	}

	while ((entry = readdir(dirp)) != NULL) {
		if (check_if_number(entry->d_name)) {
			strcpy(path, "/proc/");
			strcat(path, entry->d_name);
			strcat(path, "/comm");

			/* A file may not exist, it may have been removed.
			 * due to termination of the process. Actually we need to
			 * make sure the error is actually file does not exist to
			 * be accurate.
			 */
			fp = fopen(path, "r");
			if (fp != NULL) {
				fscanf(fp, "%s", read_buf);
				if (pname.compare(read_buf) == 0) {
					/* add to list and expand list if needed */
					pidlist[pidlist_index++] = atoi(entry->d_name);
					if (pidlist_index
							== PID_LIST_BLOCK * pidlist_realloc_count) {
						pidlist_realloc_count++;
						pidlist = (int*) realloc(pidlist,
								sizeof(int) * PID_LIST_BLOCK
										* pidlist_realloc_count); //Error check
						if (pidlist == NULL) {
							return NULL;
						}
					}
				}
				fclose(fp);
			}
		}
	}

	closedir(dirp);
	pidlist[pidlist_index] = -1; /* indicates end of list */
	return pidlist;
}

void KillAllOthers(std::string name) {
	//kill any others of this name
	int *pidlist = pidof(name);	//list of pids
	int *pids = pidlist;
	//kill each pid in list (except me)
	while (*pids != -1) {
		if (*pids != getpid())	//don't kill me
				{
			kill(*pids, SIGTERM);
			DEBUGPRINT("Killed pid %i (%s)", *pids, name.c_str());
		}
		pids++;
	}
	free(pidlist);
}

