
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>

#include "self_program.h"

#define PAGE_ADDR_MASK  (APP_SECTION_PAGE_SIZE-1)

static uint16_t CurrentPage;
static bool PageDirty = false;

//--------------------------------------------------------------------------------------------------
void flash_write(const uint8_t *buf, size_t len, uint32_t addr){
    
    static union{
        uint8_t byte[2];
        uint16_t word;
    } flash_word;
    
    uint16_t page;
    uint16_t page_addr;
    
    // round address & len down
    addr &= ~1UL;
    len &= ~1UL;
    
    while(len){
        page = addr/APP_SECTION_PAGE_SIZE;
        
        if((page != CurrentPage) && PageDirty){
            // commit the previous page 
            uint32_t address;
        
            address = (uint32_t)CurrentPage * APP_SECTION_PAGE_SIZE;
            sp_erase_write_app_page(address);
            
            CurrentPage = page;
        }
        
        page_addr = addr & PAGE_ADDR_MASK; 
        
        if((page_addr & 1UL) == 0){
            // lower byte.
            flash_word.byte[0] = *buf;
        }else{
            // odd address. load to upper part of word and commit
            flash_word.byte[1] = *buf;
            sp_load_flash_buffer(page_addr, flash_word.word);
        }
        buf++;
        addr += 1;
        len -= 1;
        PageDirty = true;
    }
}


//--------------------------------------------------------------------------------------------------
void flash_flush(void){
    if(PageDirty){
        uint32_t address;
        
        address = (uint32_t)CurrentPage * APP_SECTION_PAGE_SIZE;
        sp_erase_write_app_page(address);
        
        PageDirty = false;
    }
}

//--------------------------------------------------------------------------------------------------
void flash_erase_page(uint16_t page){
	// Calculate actual start address of the page.
	uint32_t address;

    address = (uint32_t)page * APP_SECTION_PAGE_SIZE;
	sp_erase_app_page(address);
}
