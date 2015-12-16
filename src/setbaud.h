/*
 * ATxmega version of AVR Libc's util/setbaud.h
 * 
 * The original setbaud.h include file provided by AVR libc does not seem to be compatible
 * 	with the UART in the ATxmegas. This include implements essentially the same thing
 * 
 * This is an inline include!
*/



#ifndef F_CPU
#  error "setbaud.h requires F_CPU to be defined"
#endif

#ifndef BAUD
#  error "setbaud.h requires BAUD to be defined"
#endif

#if !(F_CPU)
#  error "F_CPU must be a constant value"
#endif

#if !(BAUD)
#  error "BAUD must be a constant value"
#endif


#if defined(__DOXYGEN__)

/**
   \def BAUDCTRLA_VALUE

   Output macro from setbaud.h

   Contains the calculated baud rate control register value for BAUDCTRLA
*/
#define BAUDCTRLB_VALUE

/**
   \def BAUDCTRLB_VALUE

   Output macro from setbaud.h

   Contains the calculated baud rate control register value for BAUDCTRLB
*/
#define BAUDCTRLB_VALUE

/**
   \def USE_2X

   Output macro from setbaud.h

   Contains the value 1 if the desired baud rate setting could only
   be achieved by setting the CLK2X bit in the UART CTRLB register.
   Contains 0 otherwise.
*/
#define USE_2X


#else  /* !__DOXYGEN__ */
//==================================================================================================
//==================================================================================================

// Cleanup any dangling defines from a previous use
#undef __SB_EXP
#undef __SB_DIV
#undef USE_2X
#undef BAUDCTRLA_VALUE
#undef BAUDCTRLB_VALUE


//--------------------------------------------------------------------------------------------------
#if(BAUD > (F_CPU/64))
	// baud rate is high compared to F_CPU. Probably better results using CLK2X
	#define USE_2X	1
	
	#if(BAUD > (F_CPU/8L))
		#warning Cannot achieve requested baud rate: Baud rate is too high. (2x)
	#endif

	#if(BAUD < (F_CPU/4194304L))
		#warning Cannot achieve requested baud rate: Baud rate is too low. (2x)
	#endif

	#if(((F_CPU)/(BAUD)) < 0x000000FFUL)
		#define __SB_EXP	 -7
	#elif(((F_CPU)/(BAUD)) < 0x000001FFUL)
		#define __SB_EXP	 -6
	#elif(((F_CPU)/(BAUD)) < 0x000003FFUL)
		#define __SB_EXP	 -5
	#elif(((F_CPU)/(BAUD)) < 0x000007FFUL)
		#define __SB_EXP	 -4
	#elif(((F_CPU)/(BAUD)) < 0x00000FFFUL)
		#define __SB_EXP	 -3
	#elif(((F_CPU)/(BAUD)) < 0x00001FFEUL)
		#define __SB_EXP	 -2
	#elif(((F_CPU)/(BAUD)) < 0x00003FFCUL)
		#define __SB_EXP	 -1
	#elif(((F_CPU)/(BAUD)) < 0x00007FF8UL)
		#define __SB_EXP	 0
	#elif(((F_CPU)/(BAUD)) < 0x0000FFF0UL)
		#define __SB_EXP	 1
	#elif(((F_CPU)/(BAUD)) < 0x0001FFE0UL)
		#define __SB_EXP	 2
	#elif(((F_CPU)/(BAUD)) < 0x0003FFC0UL)
		#define __SB_EXP	 3
	#elif(((F_CPU)/(BAUD)) < 0x0007FF80UL)
		#define __SB_EXP	 4
	#elif(((F_CPU)/(BAUD)) < 0x000FFF00UL)
		#define __SB_EXP	 5
	#elif(((F_CPU)/(BAUD)) < 0x001FFE00UL)
		#define __SB_EXP	 6
	#else
		#define __SB_EXP	 7
	#endif

	#if(__SB_EXP <= -3)
		#define __SB_DIV ((((F_CPU - (8 * (BAUD))) << (-__SB_EXP - 3)) + (BAUD) / 2) / (BAUD))
	#elif(__SB_EXP < 0)
		#define __SB_DIV (((F_CPU - (8 * (BAUD))) + ((BAUD) << (__SB_EXP + 3)) / 2) / ((BAUD) << (__SB_EXP + 3)))
	#else
		#define __SB_DIV ((F_CPU + ((BAUD) << (__SB_EXP + 3)) / 2) / ((BAUD) << (__SB_EXP + 3)) - 1)
	#endif
//--------------------------------------------------------------------------------------------------
#else
	// without CLK2X
	#define USE_2X	0
	
	#if(BAUD > (F_CPU/16L))
		#warning Cannot achieve requested baud rate: Baud rate is too high.
	#endif

	#if(BAUD < (F_CPU/8388608L))
		#warning Cannot achieve requested baud rate: Baud rate is too low.
	#endif
	
	#if(((F_CPU)/((BAUD)*2)) < 0x000000FFUL)
		#define __SB_EXP	 -7
	#elif(((F_CPU)/((BAUD)*2)) < 0x000001FFUL)
		#define __SB_EXP	 -6
	#elif(((F_CPU)/((BAUD)*2)) < 0x000003FFUL)
		#define __SB_EXP	 -5
	#elif(((F_CPU)/((BAUD)*2)) < 0x000007FFUL)
		#define __SB_EXP	 -4
	#elif(((F_CPU)/((BAUD)*2)) < 0x00000FFFUL)
		#define __SB_EXP	 -3
	#elif(((F_CPU)/((BAUD)*2)) < 0x00001FFEUL)
		#define __SB_EXP	 -2
	#elif(((F_CPU)/((BAUD)*2)) < 0x00003FFCUL)
		#define __SB_EXP	 -1
	#elif(((F_CPU)/((BAUD)*2)) < 0x00007FF8UL)
		#define __SB_EXP	 0
	#elif(((F_CPU)/((BAUD)*2)) < 0x0000FFF0UL)
		#define __SB_EXP	 1
	#elif(((F_CPU)/((BAUD)*2)) < 0x0001FFE0UL)
		#define __SB_EXP	 2
	#elif(((F_CPU)/((BAUD)*2)) < 0x0003FFC0UL)
		#define __SB_EXP	 3
	#elif(((F_CPU)/((BAUD)*2)) < 0x0007FF80UL)
		#define __SB_EXP	 4
	#elif(((F_CPU)/((BAUD)*2)) < 0x000FFF00UL)
		#define __SB_EXP	 5
	#elif(((F_CPU)/((BAUD)*2)) < 0x001FFE00UL)
		#define __SB_EXP	 6
	#else
		#define __SB_EXP	 7
	#endif

	#if(__SB_EXP <= -3)
		#define __SB_DIV ((((F_CPU - (8 * ((BAUD)*2))) << (-__SB_EXP - 3)) + ((BAUD)*2) / 2) / ((BAUD)*2))
	#elif(__SB_EXP < 0)
		#define __SB_DIV (((F_CPU - (8 * ((BAUD)*2))) + (((BAUD)*2) << (__SB_EXP + 3)) / 2) / (((BAUD)*2) << (__SB_EXP + 3)))
	#else
		#define __SB_DIV ((F_CPU + (((BAUD)*2) << (__SB_EXP + 3)) / 2) / (((BAUD)*2) << (__SB_EXP + 3)) - 1)
	#endif
#endif

#define BAUDCTRLA_VALUE		((unsigned char)((__SB_DIV) & 0xFFL))
#define BAUDCTRLB_VALUE		((unsigned char)((((__SB_DIV) >> 8) & 0x0F) | ((__SB_EXP) << 4)))

#endif
