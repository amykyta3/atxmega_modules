
#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

typedef struct{
  uint8_t slave_addr;   ///< Chip address of slave device
  uint8_t addr[3];      ///< register/command payload
  uint8_t addr_len;     ///< Number of addr bytes to use (1-3)
  uint8_t *data;        ///< Pointer to data transfer buffer
  uint16_t data_len;    ///< Number of bytes to transfer
  bool read;            ///< Indicates if the transfer is a read operation or not.
} i2c_package_t;

typedef enum {
    I2C_IDLE,    ///< Bus is idle. Ready for new transfer.
    I2C_BUSY,    ///< A transfer is in progress.
    I2C_FAILED   ///< Previous transfer failed. Ready for new transfer.
} i2c_status_t;

/**
 * \brief Initialize the I2C master module
 **/
void i2c_init(void);

/**
 * \brief Uninitialize the I2C master module
 **/
void i2c_uninit(void);

/**
 * \brief Start an I2C transfer
 * 
 * This function begins a new I2C transaction as described by the \c pkg struct. This function
 * is nonblocking and returns immediately after the transfer is started. The status of the transfer 
 * can be polled using the i2c_transfer_status() function. Alternatively, a \c callback function can
 * be executed when the transfer completes.
 * 
 * \note Global interrupts must be enabled.
 * 
 * \note The \ref i2c_package_t struct \c pkg, as well as any referenced data \e must remain
 * allocated throughout the duration of the I2C transaction.
 * 
 * \param pkg        Pointer to a package struct that describes the transfer operation.
 * \param callback   Optional pointer to a callback function to execute once the transfer completes
 *                   or fails. A NULL pointer disables the callback.
 **/
void i2c_transfer_start(const i2c_package_t *pkg, void (*callback)(i2c_status_t result));

/**
 * \brief Get the status of the I2C module
 * \return status of the bus.
 **/
i2c_status_t i2c_transfer_status(void);

#endif
