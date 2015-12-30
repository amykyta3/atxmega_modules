/**
* \file
* \brief Include file for the UART driver
* \author Alex Mykyta 
**/

#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

//==================================================================================================
// Function Prototypes
//==================================================================================================

/**
* \brief Initializes the UART controller
* \attention The initialization routine does \e not setup the IO ports!
**/
void uart_init(void);

/**
* \brief Uninitializes the UART controller
**/
void uart_uninit(void);

//==================================================================================================
//                                          RX Functions
//==================================================================================================

/**
* \brief Get the number of bytes available to be read
* \return Number of bytes
**/
size_t uart_rdcount(void);

/**
* \brief Discards any data received
**/
void uart_rdflush(void);

/**
* \brief Reads the next character received from the UART
* \details If a character is not immediately available, function will block until it receives one.
* \return The next available character
**/
char uart_getc(void);

/**
* \brief Read data from the UART. Blocks until all data has been received.
* \param buf Destination buffer of the data to be read. A \c NULL pointer discards the data.
* \param size Number of bytes to be read.
**/
void uart_read(void *buf, size_t size);

//==================================================================================================
//                                          TX Functions
//==================================================================================================

/**
* \brief Transmit data over UART
* \param data Pointer to the data to be transmitted
* \param len number of bytes to send
**/
void uart_write(void *data, size_t len);

/**
* \brief Writes a character to the UART
* \param c character to be written
**/
void uart_putc(const char c);

/**
* \brief Writes a character string to the UART
* \param s Pointer to the Null-terminated string to be sent
**/
void uart_puts(const char *s);

#ifdef __cplusplus
}
#endif

#endif
