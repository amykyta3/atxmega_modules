/**
* \file
* \brief UART driver
* \author Alex Mykyta 
* 
**/


/*==================================================================================================
* File History:
* NAME          DATE         COMMENTS
* Alex M.       2013-10-09   born
* Alex M.       2015-12-16   Adapted for AVR XMega
* 
*=================================================================================================*/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "uart_io.h"

#include <uart_io_config.h>

//==================================================================================================
// Preprocessor computations
//==================================================================================================

// Resolve Modes
#if(UART_RX_MODE == 1)
    #define RXMODE_INTR
#elif(UART_RX_MODE == 2)
    #define RXMODE_DMA
#else
    #define RXMODE_POLL
#endif

#if(UART_TX_MODE == 1)
    #define TXMODE_INTR
#elif(UART_TX_MODE == 2)
    #define TXMODE_DMA
#else
    #define TXMODE_POLL
#endif

#if defined(RXMODE_INTR) || defined(TXMODE_INTR)
    #include "fifo.h"
#endif

#ifdef TXMODE_INTR
    #if(TX_FLOW_CONTROL_EN == 1)
        #define TX_FLOW_CTL
        // Generate register defines:
        // PORT_INT0LVL_LO_gc PORT_INT0LVL_MED_gc PORT_INT0LVL_HI_gc
        // PORT_INT1LVL_LO_gc PORT_INT1LVL_MED_gc PORT_INT1LVL_HI_gc
        // PORT_INT0LVL_gm PORT_INT1LVL_gm
        // INT0MASK INT1MASK
        // VPORT_INT0IF_bm VPORT_INT1IF_bm
        #define TXFC_PIN_bm (1 << TX_FLOW_PIN)
        #if(TX_FLOW_INTERRUPT_ID == 0)
            #define TXFC_INTMASK    INT0MASK
            #define TXFC_INTIF_bm   PORT_INT0IF_bm
            #define TXFC_INTLVL_gm  PORT_INT0LVL_gm
            #if(TX_FLOW_INTERRUPT_LEVEL == 1)
                #define TXFC_INTLVL_gc  PORT_INT0LVL_LO_gc
            #elif(TX_FLOW_INTERRUPT_LEVEL == 2)
                #define TXFC_INTLVL_gc  PORT_INT0LVL_MED_gc
            #else
                #define TXFC_INTLVL_gc  PORT_INT0LVL_HI_gc
            #endif
        #else
            #define TXFC_INTMASK    INT1MASK
            #define TXFC_INTIF_bm   PORT_INT1IF_bm
            #define TXFC_INTLVL_gm  PORT_INT1LVL_gm
            #if(TX_FLOW_INTERRUPT_LEVEL == 1)
                #define TXFC_INTLVL_gc  PORT_INT1LVL_LO_gc
            #elif(TX_FLOW_INTERRUPT_LEVEL == 2)
                #define TXFC_INTLVL_gc  PORT_INT1LVL_MED_gc
            #else
                #define TXFC_INTLVL_gc  PORT_INT1LVL_HI_gc
            #endif
        #endif
    #endif
#endif

//==================================================================================================
// Variable Declarations
//==================================================================================================

#ifdef RXMODE_INTR
    static uint8_t rxbuf[RX_BUF_SIZE] __attribute__ ((section (".noinit")));
    static FIFO_t RXFIFO;
#endif

#ifdef RXMODE_DMA
    static uint8_t RX_Buf[RX_BUF_SIZE] __attribute__ ((section (".noinit")));
    static volatile int8_t RX_laplead; // Number of buffer laps the DMA wridx is leading RX_rdidx by. Should be 0 or 1
    static uint8_t RX_rdidx;
#endif

#ifdef TXMODE_INTR
    static uint8_t txbuf[TX_BUF_SIZE] __attribute__ ((section (".noinit")));
    static FIFO_t TXFIFO;
#endif

#ifdef TXMODE_DMA
    static uint8_t TX_Buf[TX_BUF_SIZE] __attribute__ ((section (".noinit")));
    static uint8_t TX_transfer_idx; // Start index of the current DMA transfer
    static uint8_t TX_transfer_len; // Length of the current DMA transfer
    static uint8_t TX_wridx;
#endif

//==================================================================================================
// Functions
//==================================================================================================

void uart_init(void){
    // Clear UART
    UART_DEV.CTRLA = 0;
    UART_DEV.CTRLB = 0;
    
    #if defined(RXMODE_DMA) || defined(TXMODE_DMA)
        // Setup EDMA
        EDMA.CTRL = EDMA_RESET_bm;
        EDMA.CTRL = EDMA_ENABLE_bm;
    #endif
    
    #ifdef RXMODE_INTR
        fifo_init(&RXFIFO, rxbuf, sizeof(rxbuf));
        UART_DEV.CTRLA = RX_ISR_INTLVL;
    #endif
    
    #ifdef TXMODE_INTR
        fifo_init(&TXFIFO, txbuf, sizeof(txbuf));
    #endif
    
    #ifdef RXMODE_DMA
        /* Init RX DMA Channel
         * 
         * UART RX triggers a DMA transfer into RX_Buf
         * DMA automatically wraps to the beginning
         * Interrupt every time it wraps
         */
        
        EDMA.RX_DMA_CH.CTRLA = EDMA_CH_RESET_bm;
        EDMA.RX_DMA_CH.CTRLA = EDMA_CH_SINGLE_bm; // No repeat. DMA is restarted in the interrupt after each block.
        EDMA.RX_DMA_CH.CTRLB = RX_DMA_INTLVL;
        EDMA.RX_DMA_CH.ADDRCTRL = EDMA_CH_RELOAD_BLOCK_gc | EDMA_CH_DIR_INC_gc;
        EDMA.RX_DMA_CH.TRIGSRC = RX_DMA_TRIGSRC;
        EDMA.RX_DMA_CH.TRFCNTL = (RX_BUF_SIZE & 0xFF);
        EDMA.RX_DMA_CH.TRFCNTH = 0;
        EDMA.RX_DMA_CH.ADDRL = ((uintptr_t)(&RX_Buf)) & 0xFF;
        EDMA.RX_DMA_CH.ADDRH = ((uintptr_t)(&RX_Buf)) >> 8;
        
        EDMA.RX_DMA_CH.CTRLA |= EDMA_CH_ENABLE_bm;
        RX_laplead = 0;
        RX_rdidx = 0;
    #endif
    
    #ifdef TXMODE_DMA
        /* Init TX DMA Channel
         * 
         * When enabled, the TX DMA channel loads the UART transmit register and auto-increments the
         * buffer address.
         * TX DMA is only enabled when data is to be sent.
         * A DMA transfer is started after the buffer is loaded or when a previous transfer completes
         * and the buffer still contains outgoing data.
         */
        
        EDMA.TX_DMA_CH.CTRLA = EDMA_CH_RESET_bm;
        EDMA.TX_DMA_CH.CTRLA = EDMA_CH_SINGLE_bm;
        EDMA.TX_DMA_CH.CTRLB = TX_DMA_INTLVL;
        EDMA.TX_DMA_CH.ADDRCTRL = EDMA_CH_RELOAD_NONE_gc | EDMA_CH_DIR_INC_gc;
        EDMA.TX_DMA_CH.TRIGSRC = TX_DMA_TRIGSRC;
        
        TX_transfer_idx = 0;
        TX_transfer_len = 0;
        TX_wridx = 0;
    #endif
    
    // setbaud.h inline include calculates BAUDCTRL values
    #define BAUD	BAUD_RATE
    #include "setbaud.h"
    UART_DEV.BAUDCTRLA = BAUDCTRLA_VALUE;
    UART_DEV.BAUDCTRLB = BAUDCTRLB_VALUE;
    #if(USE_2X)
        UART_DEV.CTRLB = USART_CLK2X_bm;
    #else
        UART_DEV.CTRLB = 0;
    #endif
    #undef BAUD
    
    UART_DEV.CTRLC = USART_CHSIZE_8BIT_gc;
    // Enable UART!
    UART_DEV.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
    
    // Set up TX flow control
    #ifdef TX_FLOW_CTL
        TX_FLOW_PORT.DIRCLR = TXFC_PIN_bm; // ensure pin is input
        TX_FLOW_PORT.TXFC_INTMASK = 0x00;
        TX_FLOW_PORT.INTFLAGS = TXFC_INTIF_bm; // Clear flag
        
        // Set INTLVL for port
        TX_FLOW_PORT.INTCTRL &= ~TXFC_INTLVL_gm;
        TX_FLOW_PORT.INTCTRL |= TXFC_INTLVL_gc;
        
        // Set pin interrupt attributes (index pin number off of pin 0 control)
        (&(TX_FLOW_PORT.PIN0CTRL))[TX_FLOW_PIN] = PORT_ISC_LEVEL_gc;
    #endif
}

//--------------------------------------------------------------------------------------------------
void uart_uninit(void){
    #ifdef TX_FLOW_CTL
        TX_FLOW_PORT.TXFC_INTMASK = 0x00;
        TX_FLOW_PORT.INTCTRL &= ~TXFC_INTLVL_gm;
    #endif
    
    UART_DEV.CTRLA = 0;
    UART_DEV.CTRLB = 0;
    
    #ifdef RXMODE_DMA
        EDMA.RX_DMA_CH.CTRLA = EDMA_CH_RESET_bm;
    #endif
    
    #ifdef TXMODE_DMA
        EDMA.TX_DMA_CH.CTRLA = EDMA_CH_RESET_bm;
    #endif
}

//==================================================================================================
//                                          RX Functions
//==================================================================================================
#ifdef RXMODE_DMA
    ISR(RX_DMA_VECTOR){
        // RX DMA has wrapped around RX_Buf
        
        RX_laplead++;
        
        // Clear flags
        EDMA.RX_DMA_CH.CTRLB |= EDMA_CH_TRNIF_bm | EDMA_CH_ERRIF_bm;
        
        // Re-enable DMA manually because Atmel is a silly goose.
        EDMA.RX_DMA_CH.CTRLA |= EDMA_CH_ENABLE_bm;
    }
#endif

#ifdef RXMODE_INTR
    ISR(RX_ISR_VECTOR){
        uint8_t c;
        c = UART_DEV.DATA;
        fifo_write(&RXFIFO, &c, 1);
    }
#endif

//--------------------------------------------------------------------------------------------------
size_t uart_rdcount(void){
    #ifdef RXMODE_POLL
        if(UART_DEV.STATUS & USART_RXCIF_bm){
            return(1);
        }else{
            return(0);
        }
    #endif
    
    #ifdef RXMODE_INTR
        return(fifo_rdcount(&RXFIFO));
    #endif
    
    #ifdef RXMODE_DMA
        uint8_t trfcnt;
        int8_t laplead;
        uint8_t wridx;
        
        // get snapshot of DMA buffer status
        do{
            laplead = RX_laplead;
            trfcnt = EDMA.RX_DMA_CH.TRFCNTL;
        }while(laplead != RX_laplead); //if laplead changed, may be invalid. try again
        
        // Calculate DMA's wridx
        wridx = sizeof(RX_Buf) - trfcnt;
        
        if((laplead == 0) && (wridx >= RX_rdidx)){
            // Data doesn't wrap
            return(wridx - RX_rdidx);
            
        }else if(((laplead == 1) && (wridx <= RX_rdidx)) || ((laplead == 0) && (wridx < RX_rdidx))){
            // Available data wraps.
            return(wridx + sizeof(RX_Buf) - RX_rdidx);
        }else{
            // Overrun!
            
            // Move read pointer to a safe position
            RX_rdidx = wridx;
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
                RX_laplead -= laplead;
            }
            
            return(0);
        }
    #endif
}


//--------------------------------------------------------------------------------------------------
void uart_rdflush(void){
    #ifdef RXMODE_POLL
        // Clear flag
        UART_DEV.STATUS = USART_RXCIF_bm;
    #endif
    
    #ifdef RXMODE_INTR
        fifo_clear(&RXFIFO);
    #endif
    
    #ifdef RXMODE_DMA
        uint8_t trfcnt;
        uint8_t wridx;
        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
            trfcnt = EDMA.RX_DMA_CH.TRFCNTL;
        }
        
        // Calculate DMA's wridx
        wridx = sizeof(RX_Buf) - trfcnt;
        
        // Discard data by moving rdidx
        RX_rdidx = wridx;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
            RX_laplead = 0;
        }
    #endif
}

//--------------------------------------------------------------------------------------------------
char uart_getc(void){
    #ifdef RXMODE_POLL
        while(!(UART_DEV.STATUS & USART_RXCIF_bm));
        return(UART_DEV.DATA);
    #endif
    
    #ifdef RXMODE_INTR
      char c;
      while(fifo_rdcount(&RXFIFO) == 0);
      fifo_read(&RXFIFO, &c, 1);
      return(c);
    #endif
    
    #ifdef RXMODE_DMA
        char c;
        
        while(uart_rdcount() == 0);
        
        c = RX_Buf[RX_rdidx];
        
        if(RX_rdidx >= sizeof(RX_Buf)-1){
            // rdidx wraps around
            
            RX_rdidx = 0;
            
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
                RX_laplead--;
            }
            
        }else{
            // rdidx doesn't wrap
            RX_rdidx++;
        }
        
        return(c);
    #endif
}

//--------------------------------------------------------------------------------------------------
void uart_read(void *buf, size_t size){
    #ifdef RXMODE_POLL
        uint8_t* u8buf = (uint8_t*)buf;
        while(size > 0){
            while(!(UART_DEV.STATUS & USART_RXCIF_bm)); // wait until char received
            if(u8buf){
                *u8buf = UART_DEV.DATA;
                u8buf++;
            }else{
                // discard
                UART_DEV.STATUS = USART_RXCIF_bm;
            }
            size--;
        }
    #endif
    
    #ifdef RXMODE_INTR
        size_t rdcount;
        uint8_t* u8buf = (uint8_t*)buf;
        
        while(size > 0){
            // Get number of bytes that can be read.
            rdcount = fifo_rdcount(&RXFIFO);
            if(rdcount > size){
                rdcount = size;
            }
            
            if(rdcount != 0){
                if(u8buf){
                    fifo_read(&RXFIFO, u8buf, rdcount);
                    u8buf += rdcount;
                }else{
                    fifo_read(&RXFIFO, NULL, rdcount);
                }
                size -= rdcount;
            }
        }
    #endif
    
    #ifdef RXMODE_DMA
        uint8_t trfcnt;
        int8_t laplead;
        uint8_t wridx;
        uint8_t* u8buf = (uint8_t*)buf;
        uint16_t rdcount;
        
        while(size > 0){
            // get snapshot of DMA buffer status
            // This CANNOT be done with interrupts disabled as it could skew the time that laplead gets
            // incremented.
            do{
                laplead = RX_laplead;
                trfcnt = EDMA.RX_DMA_CH.TRFCNTL;
            }while(laplead != RX_laplead); //if laplead changed, may be invalid. try again
            
            // Calculate DMA's wridx
            wridx = sizeof(RX_Buf) - trfcnt;
            
            if((laplead == 0) && (wridx >= RX_rdidx)){
                // Data doesn't wrap
                rdcount = wridx - RX_rdidx;
                if(rdcount > size){
                    rdcount = size;
                }
                
                // copy rdcount into u8buf
                memcpy(u8buf, &rxbuf[RX_rdidx], rdcount);
                
                u8buf += rdcount;
                size -= rdcount;
                RX_rdidx += rdcount;
                
            }else if(((laplead == 1) && (wridx <= RX_rdidx)) || ((laplead == 0) && (wridx < RX_rdidx))){
                // Available data wraps.
                
                // number of bytes to the end of the buffer
                rdcount = sizeof(rxbuf) - RX_rdidx;
                if(rdcount > size){
                    rdcount = size;
                }
                
                // copy rdcount into u8buf
                memcpy(u8buf, &rxbuf[RX_rdidx], rdcount);
                
                u8buf += rdcount;
                size -= rdcount;
                RX_rdidx += rdcount;
                
                if(RX_rdidx == sizeof(rxbuf)){
                    // read to the end. Wrap back
                    RX_rdidx = 0;
                    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
                        RX_laplead--;
                    }
                }
            }else{
                // Overrun!
                
                // Move read pointer to a safe position
                RX_rdidx = wridx;
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
                    RX_laplead -= laplead;
                }
                
                // Abort reading.
                return;
            }
        }
    #endif
}

//==================================================================================================
//                                          TX Functions
//==================================================================================================

#ifdef TXMODE_DMA
    static void tx_dma_start(void){
        
        // Clear DMA flags
        EDMA.TX_DMA_CH.CTRLB |= EDMA_CH_TRNIF_bm | EDMA_CH_ERRIF_bm;
        
        // Calculate the new start index
        TX_transfer_idx = TX_transfer_idx + TX_transfer_len;
        if(TX_transfer_idx >= sizeof(TX_Buf)){
            TX_transfer_idx = 0; 
        }
        
        // Calculate TX_transfer_len
        if(TX_transfer_idx <= TX_wridx){
            // TX data is continuous
            TX_transfer_len = TX_wridx - TX_transfer_idx;
        }else{
            // TX Data wraps. Transmit first half.
            TX_transfer_len = sizeof(TX_Buf) - TX_transfer_idx;
        }
        
        // If data is to be sent, start a transfer
        if(TX_transfer_len != 0){
            EDMA.TX_DMA_CH.TRFCNTL = TX_transfer_len;
            EDMA.TX_DMA_CH.ADDRL = ((uintptr_t)(&TX_Buf[TX_transfer_idx])) & 0xFF;
            EDMA.TX_DMA_CH.ADDRH = ((uintptr_t)(&TX_Buf[TX_transfer_idx])) >> 8;
            
            EDMA.TX_DMA_CH.CTRLA |= EDMA_CH_ENABLE_bm;
        }
        
    }
    
    ISR(TX_DMA_VECTOR){
        tx_dma_start();
    }
#endif

#ifdef TXMODE_INTR
    ISR(TX_ISR_VECTOR){
        uint8_t c;
        #ifdef TX_FLOW_CTL
            if(TX_FLOW_PORT.IN & TXFC_PIN_bm){
                // CTS is high, requesting TX stop.
                
                // disable tx interrupt
                UART_DEV.CTRLA &= ~(USART_DREINTLVL_gm);
                
                // Enable pin interrupt when CTS = 0
                TX_FLOW_PORT.TXFC_INTMASK = TXFC_PIN_bm;
            }else{
                if(fifo_read(&TXFIFO, &c, 1) == 0){
                    UART_DEV.DATA = c;
                }else{
                    // disable tx interrupt
                    UART_DEV.CTRLA &= ~(USART_DREINTLVL_gm);
                }
            }
        #else
            if(fifo_read(&TXFIFO, &c, 1) == 0){
                UART_DEV.DATA = c;
            }else{
                // disable tx interrupt
                UART_DEV.CTRLA &= ~(USART_DREINTLVL_gm);
            }
        #endif
    }
#endif

#ifdef TX_FLOW_CTL
    ISR(TX_FLOW_PIN_VECTOR){
        TX_FLOW_PORT.TXFC_INTMASK = 0x00;
        TX_FLOW_PORT.INTFLAGS = TXFC_INTIF_bm; // Clear flag
        UART_DEV.CTRLA |= TX_ISR_INTLVL; // Enable TX interrupt
    }
#endif

//--------------------------------------------------------------------------------------------------
void uart_write(void *buf, size_t size){
    #ifdef TXMODE_POLL
        while(len){
            // while outgoing data exists
            uart_putc((const char)*data);
            data++;
            len--;
        }
    #endif
    
    #ifdef TXMODE_INTR
        size_t wrcount;
        uint8_t* u8buf = (uint8_t*)buf;
        
        while(size > 0){
            // Get number of bytes that can be written.
            wrcount = fifo_wrcount(&TXFIFO);
            if(wrcount > size){
                wrcount = size;
            }
            
            if(wrcount != 0){
                fifo_write(&TXFIFO, u8buf, wrcount);
                u8buf += wrcount;
                size -= wrcount;
                // Enable TX Channel
                #ifdef TX_FLOW_CTL
                    // If not transmitting already, enable TX Interrupt
                    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
                        if((TX_FLOW_PORT.TXFC_INTMASK & TXFC_PIN_bm) == 0) {
                            // Pin interrupt isn't already enabled. Safe to enable TX interrupt
                            UART_DEV.CTRLA |= TX_ISR_INTLVL;
                        }
                    }
                    
                #else
                    // Enable TX interrupt.
                    // If TX is idle, then the interrupt should occur immediately.
                    UART_DEV.CTRLA |= TX_ISR_INTLVL;
                #endif
            }
        }
    #endif
    
    #ifdef TXMODE_DMA
        uint8_t* u8buf = (uint8_t*)buf;
        while(len){
            // while outgoing data exists
            
            // Disable the DMA interrupt so it doesn't start a new transfer before loading is complete
            EDMA.TX_DMA_CH.CTRLB = 0;
            
            if((TX_wridx != TX_transfer_idx-1) && !((TX_wridx == sizeof(TX_Buf)-1) && (TX_transfer_idx == 0))){
                // Buffer has room. Copy data
                
                TX_Buf[TX_wridx] = *u8buf;
                u8buf++;
                len--;
                if(TX_wridx == sizeof(TX_Buf)-1){
                    TX_wridx = 0;
                }else{
                    TX_wridx++;
                }
            }
            
            
            // If DMA isn't running, start it
            if (!(EDMA.TX_DMA_CH.CTRLA & EDMA_CH_ENABLE_bm)){
                tx_dma_start();
            }
            
            EDMA.TX_DMA_CH.CTRLB = TX_DMA_INTLVL;
        }
    #endif
}

//--------------------------------------------------------------------------------------------------
void uart_putc(const char c){
    #ifdef TXMODE_POLL
        while(!(UART_DEV.STATUS & USART_DREIF_bm));
        UART_DEV.DATA = c;
    #endif
    
    #ifdef TXMODE_INTR
        uart_write((uint8_t*) &c, 1);
    #endif
    
    #ifdef TXMODE_DMA
        uart_write((uint8_t*) &c, 1);
    #endif
}

//--------------------------------------------------------------------------------------------------
void uart_puts(const char *s){
    #ifdef TXMODE_POLL
        while(*s){
            uart_putc(*s);
            s++;
        }
    #endif
    
    #ifdef TXMODE_INTR
        uart_write((uint8_t*)s, strlen(s));
    #endif
    
    #ifdef TXMODE_DMA
        uart_write((uint8_t*)s, strlen(s));
    #endif
}

