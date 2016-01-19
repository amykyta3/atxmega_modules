
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <avr/io.h>

//==============================================================================
// Low-level self-program commands
//==============================================================================

// Define some constants for ASM
asm(
    ".set RAMPZ,                            0x003B  ;\n"// RAMPZ address
    ".set NVM_CMD_LOAD_FLASH_BUFFER_gc,     0x23    ;\n"// Load Flash page buffer
    ".set NVM_CMD_ERASE_WRITE_APP_PAGE_gc,  0x25    ;\n"// Erase-and-write Application Section page
    ".set NVM_CMD_ERASE_APP_PAGE_gc,        0x22    ;\n"// Erase Application Section page
    ".set NVM_CMD_ERASE_APP_gc,             0x20    ;\n"// Erase Application Section
    ".set NVM_CMD,                          0x01CA  ;\n"// Address of NVM CMD register
    ".set CCP_SPM_gc,                       0x9D    ;\n"// SPM Instruction Protection
    ".set CCP,                              0x0034  ;\n"// CCP address
);

//------------------------------------------------------------------------------
void sp_load_flash_buffer(uint16_t address, uint16_t data){
    asm(
        // Save context
        "push r0                            ;\n\t"
        "push r1                            ;\n\t"
        "push r20                           ;\n\t"
        "push r30                           ;\n\t"
        "push r31                           ;\n\t"
        
        // Move data into R1:R0
        "movw r0, %1                        ;\n\t"
        
        // Move address to Z
        "movw r30, %0                       ;\n\t"
        
        // Load command into NVM Command register
        "ldi     r20, NVM_CMD_LOAD_FLASH_BUFFER_gc  ;\n\t"
        "sts     NVM_CMD, r20               ;\n\t"
        
        // Enable SPM operation
        "ldi     r20, CCP_SPM_gc            ;\n\t"
        "sts     CCP, r20                   ;\n\t"
        "spm                                ;\n\t"
        
        // Restore Context
        "pop r31                            ;\n\t"
        "pop r30                            ;\n\t"
        "pop r20                            ;\n\t"
        "pop r1                             ;\n\t"
        "pop r0                             ;\n\t"
        
        : // No outputs
        : "r"(address), "r"(data)
        : "r0", "r1", "r20", "r30", "r31"
    );
    
}

//------------------------------------------------------------------------------
void sp_erase_write_app_page(uint32_t address){
    asm(
        // Save context
        "push r19                           ;\n\t"
        "push r20                           ;\n\t"
        "push r30                           ;\n\t"
        "push r31                           ;\n\t"
        
        // Save RAMPZ
        "in  r19, RAMPZ                     ;\n\t"
        
        // Move address to Z
        "out RAMPZ, %C0                     ;\n\t"
        "movw r30, %0                       ;\n\t"
        
        // Load command into NVM Command register
        "ldi     r20, NVM_CMD_ERASE_WRITE_APP_PAGE_gc  ;\n\t"
        "sts     NVM_CMD, r20               ;\n\t"
        
        // Enable SPM operation
        "ldi     r20, CCP_SPM_gc            ;\n\t"
        "sts     CCP, r20                   ;\n\t"
        "spm                                ;\n\t"
        
        // Restore RAMPZ
        "out     RAMPZ, r19                 ;\n\t"
        
        // Restore Context
        "pop r31                            ;\n\t"
        "pop r30                            ;\n\t"
        "pop r20                            ;\n\t"
        "pop r19                            ;\n\t"
        
        : // No outputs
        : "r"(address)
        : "r19", "r20", "r30", "r31"
    );
    
    while(NVM.STATUS & NVM_NVMBUSY_bm);
}

//------------------------------------------------------------------------------
void sp_erase_app_page(uint32_t address){
    asm(
        // Save context
        "push r19                           ;\n\t"
        "push r20                           ;\n\t"
        "push r30                           ;\n\t"
        "push r31                           ;\n\t"
        
        // Save RAMPZ
        "in  r19, RAMPZ                     ;\n\t"
        
        // Move address to Z
        "out RAMPZ, %C0                     ;\n\t"
        "movw r30, %0                       ;\n\t"
        
        // Load command into NVM Command register
        "ldi     r20, NVM_CMD_ERASE_APP_PAGE_gc  ;\n\t"
        "sts     NVM_CMD, r20               ;\n\t"
        
        // Enable SPM operation
        "ldi     r20, CCP_SPM_gc            ;\n\t"
        "sts     CCP, r20                   ;\n\t"
        "spm                                ;\n\t"
        
        // Restore RAMPZ
        "out     RAMPZ, r19                 ;\n\t"
        
        // Restore Context
        "pop r31                            ;\n\t"
        "pop r30                            ;\n\t"
        "pop r20                            ;\n\t"
        "pop r19                            ;\n\t"
        
        : // No outputs
        : "r"(address)
        : "r19", "r20", "r30", "r31"
    );
    
    while(NVM.STATUS & NVM_NVMBUSY_bm);
}

//------------------------------------------------------------------------------
void sp_erase_app(void){
    asm(
        // Save context
        "push r19                           ;\n\t"
        "push r20                           ;\n\t"
        "push r30                           ;\n\t"
        "push r31                           ;\n\t"
        
        // Save RAMPZ
        "in  r19, RAMPZ                     ;\n\t"
        
        // Load an address of 0 into Z
        "clr r20                            ;\n\t"
        "out RAMPZ, r20                     ;\n\t"
        "clr r30                            ;\n\t"
        "clr r31                            ;\n\t"
        
        // Load command into NVM Command register
        "ldi     r20, NVM_CMD_ERASE_APP_gc  ;\n\t"
        "sts     NVM_CMD, r20               ;\n\t"
        
        // Enable SPM operation
        "ldi     r20, CCP_SPM_gc            ;\n\t"
        "sts     CCP, r20                   ;\n\t"
        "spm                                ;\n\t"
        
        // Restore RAMPZ
        "out     RAMPZ, r19                 ;\n\t"
        
        // Restore Context
        "pop r31                            ;\n\t"
        "pop r30                            ;\n\t"
        "pop r20                            ;\n\t"
        "pop r19                            ;\n\t"
        
        : // No outputs
        : // No inputs
        : "r19", "r20", "r30", "r31"
    );
    
    while(NVM.STATUS & NVM_NVMBUSY_bm);
}

//==============================================================================
// High-level self-program API
//==============================================================================

#define PAGE_ADDR_MASK  (APP_SECTION_PAGE_SIZE-1)

static uint16_t CurrentPage;
static bool PageDirty = false;

//------------------------------------------------------------------------------
void sp_write(const uint8_t *buf, size_t len, uint32_t addr){
    
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


//------------------------------------------------------------------------------
void sp_flush(void){
    if(PageDirty){
        uint32_t address;
        
        address = (uint32_t)CurrentPage * APP_SECTION_PAGE_SIZE;
        sp_erase_write_app_page(address);
        
        PageDirty = false;
    }
}

//------------------------------------------------------------------------------
void sp_erase_page(uint16_t page){
	// Calculate actual start address of the page.
	uint32_t address;

    address = (uint32_t)page * APP_SECTION_PAGE_SIZE;
	sp_erase_app_page(address);
}
