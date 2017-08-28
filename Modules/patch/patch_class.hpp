//
//  patch_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

//used by gateway to switch messages

#ifndef patch_class_hpp
#define patch_class_hpp

#include "ps_common.h"

#include "common/ps_root_class.hpp"
#include "serial/linux_serial/ps_serial_linux.hpp"
#include "serial/socket/ps_socket_server.hpp"
#include "packet/serial_packet/ps_packet_serial_linux.hpp"
#include "packet/xbee_packet/ps_packet_xbee_linux.hpp"
#include "packet/xbee_packet/ps_packet_xbee_class.hpp"
#include "transport/ps_transport_linux.hpp"

#include "pubsub/pubsub_header.h"

//message to communicate subscribed topics
//typedef struct {
//	ps_topic_id_t topicIds[PS_MAX_TOPIC_LIST];
//} ps_subscribe_message_t;

//struct to record subscriptions
//typedef struct {
//	message_handler_t 		*messageHandler;
//	std::set<ps_topic_id_t> topicList;
//} psClient_t;

extern const char *packet_type_names[];

class patch_class : public ps_root_class {

public:
    patch_class(ps_packet_xbee_linux *xbee_module, const char *name, int _xbee_address, int _socket_listen_port);

    bool is_online();

    int xbee_address;
    int socket_listen_port;

    //callbacks from transports
    void process_observed_data(ps_root_class *src, const void *msg, int length) override;
    void process_observed_event(ps_root_class *src, int event) override;

    //not used
    void message_handler(ps_packet_source_t packet_source,
                         ps_packet_type_t   packet_type,
                         const void *msg, int length) override {}

    FILE *patchDebugFile;
protected:
    bool online {false};
    int online_condition;

	//XBee packet object
	ps_packet_xbee_class *xbee_pkt;
	//xbee transport layer
	ps_transport_linux *xbee;

	//socket server
	ps_socket_server *socket_server;
	//socket packet layer
	ps_packet_serial_linux *socket_pkt;
	//socket transport layer
	ps_transport_linux *socket;

};


#endif /* patch_class_hpp */
