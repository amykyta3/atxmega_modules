
#ifndef SELF_PROGRAM_H
#define SELF_PROGRAM_H

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

#endif
