
#include <avr/io.h>
#include <stdint.h>

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

void sp_load_flash_buffer(uint16_t address, uint16_t data){
    asm(
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
        
        : // No outputs
        : "r"(address), "r"(data)
        : "r0", "r1", "r20", "r30", "r31"
    );
    
    while(NVM.STATUS & NVM_NVMBUSY_bm);
}


void sp_erase_write_app_page(uint32_t address){
    asm(
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
        
        : // No outputs
        : "r"(address)
        : "r19", "r20", "r30", "r31"
    );
    
    while(NVM.STATUS & NVM_NVMBUSY_bm);
}


void sp_erase_app_page(uint32_t address){
    asm(
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
        
        : // No outputs
        : "r"(address)
        : "r19", "r20", "r30", "r31"
    );
    
    while(NVM.STATUS & NVM_NVMBUSY_bm);
}


void sp_erase_app(void){
    asm(
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
        
        : // No outputs
        : //"M"(CCP_SPM_gc), "M"(NVM_CMD_ERASE_APP_gc), "I"(&RAMPZ)
        : "r19", "r20", "r30", "r31"
    );
    
    while(NVM.STATUS & NVM_NVMBUSY_bm);
}
