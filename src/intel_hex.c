

#include <stdint.h>

#include "intel_hex.h"

static uint8_t checksum = 0;

//--------------------------------------------------------------------------------------------------
static int8_t hex2nibble(char c){
    if((c >= '0') && (c <= '9')){
        return(c-'0');
    }else if((c >= 'A') && (c <= 'F')){
        return(c-'A'+10);
    }else if((c >= 'a') && (c <= 'f')){
        return(c-'a'+10);
    }else{
        return(-1);
    }
}

//--------------------------------------------------------------------------------------------------
static uint8_t hex2byte(char *str){
    uint8_t b;
    int8_t nib;
    
    nib = hex2nibble(*str++);
    if(nib < 0){
        return(0);
    }
    b = nib;
    b <<= 4;
    nib = hex2nibble(*str);
    if(nib < 0){
        return(0);
    }
    b += nib;
    checksum += b;
    return(b);
}

//--------------------------------------------------------------------------------------------------
uint8_t parse_intel_hex(char *str, struct ihex_packet *dst){
    static uint32_t base_address = 0;
    
    uint16_t addr;
    uint8_t rectype;
    
    // Start Code
    if(*str++ != ':') return(IHEX_ERROR);
    checksum = 0;
    
    // Len
    dst->len = hex2byte(str);
    str += 2;
    
    // addr
    addr = hex2byte(str);
    str += 2;
    addr <<= 8;
    addr += hex2byte(str);
    str += 2;
    
    // Record type
    rectype = hex2byte(str);
    str += 2;
    
    // Data
    if(dst->len > sizeof(dst->data)) return(IHEX_ERROR);
    for(uint8_t i=0; i<dst->len; i++){
        dst->data[i] = hex2byte(str);
        str += 2;
    }
    
    // Checksum
    hex2byte(str);
    if(checksum != 0) return(IHEX_ERROR);
    
    switch(rectype){
        case IHEX_DATA:
            dst->addr = base_address;
            dst->addr += addr;
            break;
        case IHEX_EOF:
            base_address = 0;
            break;
        case IHEX_EXT_SEGMENT:
            if(dst->len != 2) return(IHEX_ERROR);
            base_address = dst->data[0];
            base_address <<= 8;
            base_address += (dst->data[1] & 0xF0);
			base_address <<= 4;
            break;
        case IHEX_EXT_LINEAR:
            if(dst->len != 2) return(IHEX_ERROR);
            base_address = dst->data[0];
			base_address <<= 8;
			base_address += dst->data[1];
			base_address <<= 16;
            break;
        default:
            return(IHEX_ERROR);
    }
    
    return(rectype);
}
