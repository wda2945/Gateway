//
//  SoftwareProfile.h
//  Fido
//
//  Created by Martin Lane-Smith on 6/17/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//

#ifndef SoftwareProfile_h
#define SoftwareProfile_h

#define PROCESS_NAME "gateway"

#define MAIN_DEBUG

//enabled subsystem debug
#define BROKER_DEBUG
#define NOTIFICATIONS_DEBUG
#define AGENT_DEBUG

#define PING_PORT_NUMBER		5000
#define GATEWAY_XBEE_ADDRESS	0x1A
#define GATEWAY_LISTEN_PORT		6000

//XBee broker
#define PAN_ID					0x3332

#define XBEE_UART_DEVICE 	"/dev/ttyO2"
#define XBEE_TX_PIN			"P9_21"
#define XBEE_RX_PIN			"P9_22"

#define XBEE_UART_BAUDRATE 	B115200

//local XBEE pins
#define XBEE_DTR			"P8-26"
#define XBEE_DTR_GPIO		61

#define XBEE_CTS			"P8-31"
#define XBEE_CTS_GPIO		10

#define XBEE_RTS			"P8-36"
#define XBEE_RTS_GPIO		80

#define XBEE_RESET		"P9-27"			//reset pin driving (via OC transistor)
#define XBEE_RESET_GPIO	115				//high to reset

#define XBEE_STATUS			"P9-28"
#define XBEE_STATUS_GPIO	113

#define XBEE_ASSOCIATE		"P9-30"
#define XBEE_ASSOCIATE_GPIO	112

//Logging level
#define SYSLOG_LEVEL                LOG_ALL  	//published log


#endif
