#ifndef FLASH_PROGRAM_H
#define FLASH_PROGRAM_H

#include <stdint.h>

/**
 * \brief Write a data buffer to the Application Flash
 * 
 * \details Loads the user-provided data into the flash page buffer. If the write address crosses a
 * page boundary, the page buffer is committed to flash and data continues to be written to the
 * fresh page buffer.
 * 
 * \warning Multiple flash_write() operations must have sequential addresses (or at least operate
 * within one page at a time)
 * 
 * \param buf	Pointer to the data
 * \param size  Number of bytes (even number only)
 * \param addr  Write start address (byte address. Even address)
 **/
void flash_write(const uint8_t *buf, size_t len, uint32_t addr);


/**
 * \brief Commits any remaining data saved in the page buffer into application memory
 **/
void flash_flush(void);

/**
 * \brief Erase a flash page
 * 
 * \param page  Page address
 **/
void flash_erase_page(uint16_t page);

#endif
