#ifndef UART_IO_CONFIG_H
#define UART_IO_CONFIG_H

//==================================================================================================
// UART Configuration
//==================================================================================================

#define UART_RX_MODE    0
#define UART_TX_MODE    0
//  0 = Polling
//  1 = Interrupt
//  2 = DMA

// Select a UART Device
#define UART_DEV        USARTD0
#define UART_DEV_PORT   PORTD
#define UART_TXPIN      PIN3_bm

// Baud rate in Hz
#define BAUD_RATE       9600L

// CPU Clock in Hz
#define F_CPU           2000000L

// Buffer sizes. Max is 256 for DMA mode
#define RX_BUF_SIZE     64
#define TX_BUF_SIZE     64

//==================================================================================================
// TX Flow Control (Only supported if UART_TX_MODE == 1)
//==================================================================================================
#define TX_FLOW_CONTROL_EN  0
#define TX_FLOW_PORT        PORTA
#define TX_FLOW_PIN         0

// Set interrupt level
#define TX_FLOW_INTERRUPT_LEVEL 2
//  1 = Low
//  2 = Med
//  3 = High

// Set interrupt ID to use (0 or 1)
#define TX_FLOW_INTERRUPT_ID    0

// Interrupt vector for CTS pin's port
#define TX_FLOW_PIN_VECTOR      PORTA_INT0_vect

//==================================================================================================
// Interrupt mode configuration (If UART_XX_MODE == 1)
//==================================================================================================
#define RX_ISR_VECTOR   USARTD0_RXC_vect
#define TX_ISR_VECTOR   USARTD0_DRE_vect

#define RX_ISR_INTLVL   USART_RXCINTLVL_HI_gc
#define TX_ISR_INTLVL   USART_DREINTLVL_HI_gc

//==================================================================================================
// DMA Configuration (If UART_XX_MODE == 2)
//==================================================================================================

// DMA Trigger sources
#define RX_DMA_TRIGSRC  EDMA_CH_TRIGSRC_USARTD0_RXC_gc
#define TX_DMA_TRIGSRC  EDMA_CH_TRIGSRC_USARTD0_DRE_gc

// Select a DMA Channel
#define RX_DMA_CH       CH0
#define RX_DMA_VECTOR   EDMA_CH0_vect

// Select a DMA Channel
#define TX_DMA_CH       CH1
#define TX_DMA_VECTOR   EDMA_CH1_vect

// Select an interrupt priority level
#define RX_DMA_INTLVL   (EDMA_CH_TRNINTLVL1_bm | EDMA_CH_TRNINTLVL0_bm) // Highest priority
#define TX_DMA_INTLVL   (EDMA_CH_TRNINTLVL1_bm | EDMA_CH_TRNINTLVL0_bm) // Highest priority


#endif