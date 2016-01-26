/******************************************************************************
*
* UART.c
* Author: Mario Washington
*
* HW Schematic - Pg 1 - C5515GpioMmcSpi
*
**************************************************************************** */

/*
	Initialization Steps (from UART User's guide)
	1. Perform the necessary device pin multiplexing setup (EBSR)
	2.	Ensure the UART is out of reset by setting UART_RST = 0 in 
		the peripheral reset control regiser (PRCR)
	3. Enable the UART input clock by setting UARTCG to 1 in the 
		peripheral clock gating configuration register (PCGCR1). See
		the device-specific data manual for more information on PCGCR1
	4. Place the UART transmitter and receiver in reset by setting UTRST
		and URRST to 0 in the UART power and emulation management
		register (PWREMU_MGMT)
	5.	Set the desired baud rate by writing the appropriate clock 
		divisor values to the divisor latch registers (DLL and DLH)
	6.	Select the desired trigger level and enable the FIFOs by writing the
		appropriate values to the FIFO control register (FCR) if the FIFOs 
		are used. The FIFOEN bit in FCR must be set first, before the other
		bits in FCR are configured. Be sure to set the DMAMODE1 bit to 1 as
		required for proper operation between the DMA and UART.
	7. Choose the desired protocl settings by writing the appropriate values 
		to the line control register (LCR)
	8. Write appropriate values to the modem control register (MCR) if autoflow
		control is desired. Note that all UARTs do not support autoflow
		control, see the device-specific data manual for supported features.
	9. Choose the desired response to emulation suspend events by configuring
		the FREE bit and enable the UART by setting the UTRST and URRST bits in
		the power and emulation management register (PWREMU_MGMT).
*/
Uint16 Asthmagram_uart_init() {
	
	/* Check device pin multiplexing */
	if(EBSR != 0x1100) {
		/* set EBSR = 0x1100;*/
	}
	
	/* Take UART out of reset (UART_RST = 0 in PRCR)*/

	
	/* set UARTCG to 1 in PCGRCR1
	PCGRCR1 | UARTCG_ENABLE */
	
	/* Reset UART Tx & Rx */
	
	/* Set desired BAUD rate */
}