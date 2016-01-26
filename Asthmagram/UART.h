/******************************************************************************
*
* UART - BLE
* Author: Mario Washington
*
* HW Schematic - Pg 1 - C5515GpioMmcSpi
*
**************************************************************************** */

/* HW Ports:
UART_RTS - LCD_D[12]/UART_RTS/GP[28]/I2S3_CLK
UART_CTS - LCD_D[13]/UART_CTS/GP[29]/I2S3_FS
UART_RX   - LCD_D[14]/UART_RX/GP[30]/I2S3_RX
UART_TX   - LCD_D[15]/UART_DX/GP[31]/ITS3_DX
*/

/*
**<RegisterName>[<RegisterLocation>] **

EBSR [1C00h] (16-BITS)

	15		14-12	  	  11-10		9-8			7-6			
Reserved	PPMODE	SP1MODE	SP0MODE	RESERVED


   5				4				3				2			1			0
A20_MODE A19_MODE	A18_MODE   A17..	A16...	A15...

*note: all bits = 0 after reset

1. Clock Gating Control RegisterLocation (Section 1.5.3.2 )
2. Modify EBSR
3. Reset Peripherals (Peripheral Software Reset Counter Register - PSRCR)

14-12 	PPMODE: Parallel Port Mode Control Bits
		For UART:		001, 100, 101 <---- Likely using 001 because 100 and 101 use LCD
11-10	SP1MODE: Serial Port 1 Mode Control Bits
		00 <--- All 6 Signals of the MMC/SD1 routed to 6 external signals of the serial port 1
9-8		SP0MODE: Serial Port 0 Mode Control Bits
		01 <----- 4 Signals of the I2S0 module and 2 GP[5:4]
7-6		Reserved
5			A20_MODE: A20 Pin Mode Bit.
		0
4			A19_MODE: A19 Pin Mode Bit.
		0
3			A18_MODE: A18 Pin Mode Bit.
		0
2			A17_MODE: A17 Pin Mode Bit.
		0
1			A16_MODE: A16 Pin Mode Bit.
		0
0			A15_MODE: A15 Pin Mode Bit.
		0
					[15]	[14:12]	[11:10]	[9:8]	[7:6]	[5] [4] [3] [2] [1] [0]
EBSR[1C00] = 0 	   001       00        01   00     0   0   0   0    0	0
EBSR[1C00] = 0001, 0001, 0000, 0000 = 0x1100

*******************************************************************************
1.7.5.2 Peripheral Reset Control Register (PRCR) [1C05h]
Writing a 1 to any bits in this register initiates the reset sequence for the associated peripherals. The
associated peripherals will be held in reset for the duration of clock cycles set in the PSRCR register and
they should not be accessed during that time. Reads of this register return the state of the reset signal for
the associated peripherals. In other words, polling may be used to wait for the reset to become deasserted.

			Write 0 - No Effect		Write 1 - Start Reset		
			Read 0 - Out of Reset	Read 1 - Held in reset, don't access

			PRCR[1C05h] (16-Bits)
	15-8	: 	Reserved
	7		:	PG4_RST	-	Group 4 - LCD, I2S2, I2S3, UART, SPI
	6		:	Reserved
	5		:	PG3_RST	-	Group 3 - MMC/SDO, MMC/SD1, I2S0, I2S1				
	4		:	DMA_RST 	-	DMA Software Reset (All 4 controllers)
	3		:	USB_RST	-	USB Software Reset
	2		:	SAR_RST	-	SAR Software reset & most analog-related register in
									IO-space address range of 0x7000 - 0x70FF
	1		:	PG1_RST	-	Group 1 - EMIF and 3 timer reset
	0		:	I2C_RST	-	I2C Software reset
	
1.5.3.2.1 Peripheral Clock Gating Configuration Registers (PCGCR1 and PCGCR2) [1C02 - 1C03h]
The peripheral clock gating configuration registers (PCGRC1 and PCGCR2) are used to disable the clocks
of the DSP peripherals. In contrast to the idle control register (ICR), these bits take effect within 6
SYSCLK cycles and do not require an idle instruction.
The peripheral clock gating configuration register 1 (PCGCR1) is shown in Figure 1-14 and described in
Table 1-24.
	0 - Clock is Active 	1 - Clock is Disabled
	15:	SYSCLKDIS	*NOTE: Disabling SYSCLKDIS disables clock to most of DSP (& CPU)
	14:	I2S2CG			
	13:	TMR2CG		
	12:	TMR1CG		
	11:	EMIFCG			*NOTE: Request permission before stopping EMIF clock through CLKSTOP
	10:	TMR0CG		
	09:	I2S1CG
	08:	I2S0CG
	07:	MMCSD1CG
	06:	I2CCG
	05:	Reserved
	04:	MMCSD0CG
	03:	DMA0CG
	02:	UARTCG		*NOTE: Request permission before stopping UART clock through CLKSTOP
	01:	SPICG
	00:	I2S3CG

*/
/* UART Registers 
The following registers share one address:
	• 	RBR, THR, and DLL. When the DLAB bit in LCR is 0, reading from the address gives the content of
		RBR, and writing to the address modifies THR. When DLAB = 1, all accesses at the address read or
		modify DLL. DLL can also be accessed by the CPU at word address 1B10h.
	• IER and DLH. When DLAB = 0, all accesses read or modify IER. When DLAB = 1, all accesses read
		or modify DLH. DLH can also be accessed by the CPU at word address 1B12h.
	• IIR and FCR share one address. Regardless of the value of the DLAB bit, reading from the address
		gives the content of IIR, and writing modifies FCR.
 
 From Table 7
UART_RBR 					0x1B00 	//RBR Receiver Buffer Register (read only) Section 3.1
UART_THR 					0x1B00 	//THR Transmitter Holding Register (write only) Section 3.2
UART_IER 					0x1B02 	//IER Interrupt Enable Register Section 3.3
UART_IIR 						0x1B04 	//IIR Interrupt Identification Register (read only) Section 3.4
UART_FCR 					0x1B04 	//FCR FIFO Control Register (write only) Section 3.5
UART_LCR					0x1B06 	//LCR Line Control Register Section 3.6
UART_MCR					0x1B08 	//MCR Modem Control Register Section 3.7
UART_LSR						0x1B0A 	//LSR Line Status Register Section 3.8
UART_SCR					0x1B0E 	//SCR Scratch Register Section 3.9
UART_DLL						0x1B10 	//DLL Divisor LSB Latch Section 3.10
UART_DLH					0x1B12 	//DLH Divisor MSB Latch Section 3.10
UART_PWREMU_MGMT	0x1B18 	//PWREMU_MGMT Power and Emulation Management Register 
																//Section 3.11
*/
/* 	USBSTK5515 Defines these in "usbstk5515.h" 	*/
#define UART_RBR           			*(volatile ioport Uint16*)(0x1B00)
#define UART_THR           			*(volatile ioport Uint16*)(0x1B00)
#define UART_IER           			*(volatile ioport Uint16*)(0x1B02)
#define UART_IIR           			*(volatile ioport Uint16*)(0x1B04)
#define UART_FCR           			*(volatile ioport Uint16*)(0x1B04)
#define UART_LCR           			*(volatile ioport Uint16*)(0x1B06)
#define UART_MCR           			*(volatile ioport Uint16*)(0x1B08)
#define UART_LSR           			*(volatile ioport Uint16*)(0x1B0A)
#define UART_SCR           			*(volatile ioport Uint16*)(0x1B0E)
#define UART_DLL			          	*(volatile ioport Uint16*)(0x1B10)
#define UART_DLH           			*(volatile ioport Uint16*)(0x1B12)
#define UART_PWREMU_MGMT	*(volatile ioport Uint16*)(0x1B18)


#define UART_RST_MASK			0x0040 /* check in PRCR */
#define UART_CG_MASK			0x0002 /* check in PCGCR1 */
#define UART_UTRST_MASK		0xE000 /* check in PWREMU_MGMT */
#define UART_URRST_MASK		0xD000 /* check in PWREMU_MGMT*/
#define UART_FREE_MASK			0x0001 /* check in PWREMU_MGMT */

