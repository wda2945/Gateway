/*
 * File:   uart.h
 * Author: martin
 *
 * Created on August 7, 2013, 8:19 PM
 */

#ifndef UART_H
#define	UART_H

#ifdef __cplusplus
extern "C" {
#endif

int uart_setup(const char *txpin, const char *rxpin);

#ifdef __cplusplus
}
#endif

#endif
