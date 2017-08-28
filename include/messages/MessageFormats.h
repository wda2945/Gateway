/* 
 * File:   MessageFormats.h
 * Author: martin
 *
 * Created on December 8, 2013, 10:53 AM
 */

#ifndef MESSAGEFORMATS_H
#define	MESSAGEFORMATS_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

//----------------------SYSTEM MANAGEMENT

//PS_TICK_PAYLOAD

//Data broadcast by 1S tick
typedef struct {
    uint8_t systemPowerState;		//PowerState_enum
} psTickPayload_t;



#endif	/* MESSAGEFORMATS_H */

