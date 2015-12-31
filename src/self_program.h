
#ifndef SELF_PROGRAM_H
#define SELF_PROGRAM_H

#include <stdint.h>

//==============================================================================
// Low-level self-program commands
//==============================================================================
/**
* \brief Write a word from \c data into the Flash page buffer at \c address.
* 
* \param address Position in inside the flash page buffer.
* \param data    Value to be put into the buffer.
**/
void sp_load_flash_buffer(uint16_t address, uint16_t data);

/**
* \brief Erase, then write the page buffer to the Flash page at \c address
*  in the application section. The address can point anywhere inside the page.
* 
* \param address Byte address for flash page.
**/
void sp_erase_write_app_page(uint32_t address);

/**
* \brief Erase the page at \c address in the application flash section.
*  The address can point anywhere inside the page.
* 
* \param address Byte address for flash page.
**/
void sp_erase_app_page(uint32_t address);

/**
* \brief Erase the entire application Flash section
**/
void sp_erase_app(void);

//==============================================================================
// High-level self-program API
//==============================================================================
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
void sp_write(const uint8_t *buf, size_t len, uint32_t addr);


/**
* \brief Commits any remaining data saved in the page buffer into application memory
**/
void sp_flush(void);

/**
* \brief Erase a flash page
* 
* \param page  Page address
**/
void sp_erase_page(uint16_t page);

#endif
