
#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

// Select which TWI module to use
#define I2C_DEV         TWIC
#define I2C_IRQ_vect    TWIC_TWIM_vect

#define I2C_INTLVL      TWI_MASTER_INTLVL_MED_gc

// Value of baud rate register
#define I2C_BAUD_REG    155 // For F_CPU = 32 MHz --> F_SPI = 100 kHz

#endif
