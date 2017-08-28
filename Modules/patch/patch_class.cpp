//
//  ps_pubsub_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "string.h"

#include "gateway.h"
#include "software_profile.h"
#include "ps.h"
#include "ps_config.h"
#include "ps_common.h"
#include "patch_class.hpp"

#include "pubsub/ps_pubsub_class.hpp"
#include "syslog/ps_syslog_linux.hpp"
#include "registry/ps_registry_message.h"

extern const char *topic_names[];
extern const char *pkt_type_names[];

#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(patchDebugFile, __VA_ARGS__);

int next_condition = CONDITIONS_COUNT;

patch_class::patch_class(ps_packet_xbee_linux *xbee_module, const char *name, int _xbee_address, int _socket_listen_port)
	: ps_root_class(name)
{
	patchDebugFile = fopen_logfile(name);

	xbee_address		= _xbee_address;
	socket_listen_port	= _socket_listen_port;
	online_condition	= next_condition++;

	DEBUGPRINT("patch: max_packet = %d", MAX_TRANSPORT_PACKET);

	char transName[50];

	//XBee packet object
	snprintf(transName, 50, "%s_xbeepkt", name);
	xbee_pkt = new ps_packet_xbee_class(transName, xbee_module, xbee_address);
	//xbee transport layer

	snprintf(transName, 50, "%s_xbeetran", name);
	xbee = new ps_transport_linux(transName, xbee_pkt);
	xbee->add_data_observer(this);
	xbee->add_event_observer(this);

	//socket server
	snprintf(transName, 50, "%s_sockpkt", name);
	socket_server = new ps_socket_server(socket_listen_port, 0);
	//socket packet layer
	socket_pkt = new ps_packet_serial_linux(transName, socket_server);
	//socket transport layer
	snprintf(transName, 50, "%s_socktran", name);
	socket = new ps_transport_linux(transName, socket_pkt);
//	socket->transport_source = SRC_IOSAPP;
	socket->add_data_observer(this);
	socket->add_event_observer(this);

	char reg_name[100];

	snprintf(reg_name, 100, "%s xbee address", name);
	ps_registry_add_new("Robots", reg_name, PS_REGISTRY_INT_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_set_int("Robots", reg_name, xbee_address);

	snprintf(reg_name, 100, "%s socket port", name);
	ps_registry_add_new("Robots", reg_name, PS_REGISTRY_INT_TYPE, PS_REGISTRY_SRC_WRITE);
	ps_registry_set_int("Robots", reg_name, socket_listen_port);

	snprintf(reg_name, 100, "%s online", name);
	register_condition("Conditions", online_condition, reg_name);

	DEBUGPRINT("%s patch started", name);
}

bool patch_class::is_online()
{
	return xbee->is_online();
}

//////////////////////////transport api

//transport callbacks, data and events

void patch_class::process_observed_data(ps_root_class *_pst, const void *message, int len)
{
    ps_transport_class *pst = dynamic_cast<ps_transport_class *>(_pst);
    
//    DEBUGPRINT("patch: process_observed_data %s", pst->name.c_str())

    //incoming from Transport
    const pubsub_packet_t *packet = static_cast<const pubsub_packet_t*>(message);
    
//    pst->transport_source = static_cast<Source_t>(prefix->packet_source);
    
    if (packet->prefix.packet_type < PACKET_TYPES_COUNT)
    {
        if (packet->prefix.packet_type == PUBLISH_PACKET)
        {
        	DEBUGPRINT("patch: rx topic %i from %s", packet->prefix.topic_id, pst->name.c_str());
        }
        else if (packet->prefix.packet_type == SYSLOG_PACKET && pst == xbee)
        {
        	ps_syslog_message_t *log_msg = (ps_syslog_message_t *) packet->message;
        	ps_syslog_linux &psl = dynamic_cast<ps_syslog_linux &>(the_logger());
        	psl.print_log_message(log_msg);
        }
        else
        {
        	DEBUGPRINT("patch: rx %s from %s",pkt_type_names[packet->prefix.packet_type], pst->name.c_str());
        }
    }
    else
    {
    	DEBUGPRINT("patch: rx bad pkt %i from %s", packet->prefix.packet_type, pst->name.c_str());
    }

    if (pst == xbee)
    {
    	socket->send_packet(message, len);
    }
    else
    {
    	xbee->send_packet(message, len);
    }
}

void patch_class::process_observed_event(ps_root_class *_pst, int _ev)
{
    ps_transport_class *pst = dynamic_cast<ps_transport_class *>(_pst);
    ps_transport_event_enum ev = (ps_transport_event_enum) _ev;
    
    PS_DEBUG("pub: status %s from %s", transport_event_names[ev], pst->name.c_str());
    
    switch(ev)
    {
        case PS_TRANSPORT_ONLINE:
        	if (pst == xbee) {
        		online = true;
        		ps_set_condition(online_condition);
        		LogInfo("gateway: %s online", name.c_str());
        	}
        	else
        	{
        		LogInfo("gateway: %s client connected", name.c_str());
        		ps_set_condition(CLIENT_CONNECTED);
        	}
            break;
        case PS_TRANSPORT_OFFLINE:
        	if (pst == xbee) {
        		online = false;
        		ps_cancel_condition(online_condition);
        		LogInfo("gateway: %s offline", name.c_str());
        	}
        	else
        	{
        		LogInfo("gateway: %s client disconnected", name.c_str());
        		ps_cancel_condition(CLIENT_CONNECTED);
        	}
            break;
        default:
            break;
    }
}

#define topicmacro(e, name) name,
const char *topic_names[] = {
#include "/Users/martin/Dropbox/RoboticsCodebase/RobotMonitor/rm_messages/rm_topics_list.h"
#include "messages/topics_list.h"
};
#undef topicmacro

#define packet_macro(e, name, qos) name,
const char *pkt_type_names[] = {
#include "common/ps_packet_macros.h"
};
#undef packet_macro
