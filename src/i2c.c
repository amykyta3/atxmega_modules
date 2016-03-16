/*
* Copyright (c) 2016, Alex Mykyta
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met: 
* 
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer. 
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*==================================================================================================
* File History:
* NAME          DATE         COMMENTS
* Alex M.       2016-03-08   Translated from msp430 --> atxmega
* 
*=================================================================================================*/

#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "i2c.h"
#include <i2c_internal.h>

//--------------------------------------------------------------------------------------------------

static struct{
    i2c_package_t    *pkg;
    uint16_t         idx;
    bool             sending_addr;
    void (*callback)(i2c_status_t result);
    i2c_status_t     status;
} transfer;

//--------------------------------------------------------------------------------------------------
ISR(I2C_IRQ_vect) {
    uint8_t const status = I2C_DEV.MASTER.STATUS;
    
    // Clear all flags
    I2C_DEV.MASTER.STATUS |= TWI_MASTER_RIF_bm | TWI_MASTER_WIF_bm | TWI_MASTER_CLKHOLD_bm
                            | TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm;
    
    if(transfer.pkg = NULL) return;
    
    if(status & (TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm | TWI_MASTER_RXACK_bm)){
        // Error occurred
        I2C_DEV.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        transfer.status = I2C_FAILED;
        if(transfer.callback){
            transfer.callback(I2C_FAILED);
        }
        
    
    } else if(status & TWI_MASTER_WIF_bm) { //--------------------------
        // Write interrupt. Ready to send another byte
        
        if(transfer.sending_addr){
            if(transfer.idx < transfer.pkg->addr_len) {
                // Send Byte
                I2C_DEV.MASTER.DATA = transfer.pkg->addr[transfer.idx];
                transfer.idx++;
            } else {
                // Done sending address
                transfer.sending_addr = false;
                transfer.idx = 0;
                
                if(transfer.pkg->read){
                    // Re-start condition to do a read
                    I2C_DEV.MASTER.ADDR |= 0x01;
                }
            }
        }
        
        if(!transfer.sending_addr){
            if(transfer.idx < transfer.pkg->data_len) {
                // Send Byte
                I2C_DEV.MASTER.DATA = transfer.pkg->data[transfer.idx];
                transfer.idx++;
            } else {
                // Done with transfer
                I2C_DEV.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
                transfer.status = I2C_IDLE;
                if(transfer.callback){
                    transfer.callback(I2C_IDLE);
                }
            }
        }
        
        
    } else if(status & TWI_MASTER_RIF_bm) { //--------------------------
        // Read interrupt. Finished receiving a byte
        if(transfer.idx < transfer.pkg->data_len){
            transfer.pkg->data[transfer.idx];
            transfer.idx++;
        }else{
            // Nowhere to put rd data. Stop
            I2C_DEV.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
            transfer.status = I2C_IDLE;
            if(transfer.callback){
                transfer.callback(I2C_IDLE);
            }
        }
        
        // Receive another byte?
        if(transfer.idx < transfer.pkg->data_len){
            I2C_DEV.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
        }else{
            // Done
            I2C_DEV.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
            transfer.status = I2C_IDLE;
            if(transfer.callback){
                transfer.callback(I2C_IDLE);
            }
        }
        
    }
}

//--------------------------------------------------------------------------------------------------
void i2c_init(void){
    
    I2C_DEV.MASTER.BAUD = I2C_BAUD_REG;
    I2C_DEV.MASTER.CTRLA = I2C_INTLVL | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_ENABLE_bm;
    I2C_DEV.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc | TWI_MASTER_RIF_bm 
                            | TWI_MASTER_WIF_bm | TWI_MASTER_CLKHOLD_bm
                            | TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm;
    
    transfer.status = I2C_IDLE;
}

//--------------------------------------------------------------------------------------------------
void i2c_uninit(void){
    I2C_DEV.MASTER.CTRLA = 0;
}

//--------------------------------------------------------------------------------------------------
void i2c_transfer_start(const i2c_package_t *pkg, void (*callback)(i2c_status_t result)){
    
    if(i2c_transfer_status() == I2C_BUSY) return;
    
    transfer.pkg = (i2c_package_t*)pkg;
    transfer.idx = 0;
    transfer.callback = callback;
    transfer.status = I2C_BUSY;
    
    if((pkg->addr_len == 0) && (pkg->data_len == 0)){
        // No payload. Issuing a "quick-command"
        I2C_DEV.MASTER.CTRLB = TWI_MASTER_QCEN_bm;
    }else{
        I2C_DEV.MASTER.CTRLB = 0;
    }
    
    if(pkg->addr_len > 0){
        // Need to write register address first.
        transfer.sending_addr = true;
        I2C_DEV.MASTER.ADDR = (pkg->slave_addr << 1);
    } else {
        // No address stage. Data transfer only
        transfer.sending_addr = false;
        
        if(pkg->read){
            I2C_DEV.MASTER.ADDR = (pkg->slave_addr << 1) | 0x01;
        }else{
            I2C_DEV.MASTER.ADDR = (pkg->slave_addr << 1);
        }
    }
}

//--------------------------------------------------------------------------------------------------
i2c_status_t i2c_transfer_status(void){
    i2c_status_t status;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        status = transfer.status;
    }
    
    return(status);
}