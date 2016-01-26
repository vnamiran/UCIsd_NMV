/*
*	Filename:		Asthmagram.h
*	Author:			Mario Washington
*	Description:	Register definitions & prototypes
*/

/*
*
*
*/

#define SYS_EXBUSSEL       		*(volatile ioport Uint16*)(0x1c00) // EBSR
#define SYS_PCGCR1         		*(volatile ioport Uint16*)(0x1c02) // PCGCR1
#define SYS_PCGCR2         		*(volatile ioport Uint16*)(0x1c03) // PSRCR2
#define SYS_PRCNTR         		*(volatile ioport Uint16*)(0x1c04) // PSRCR
#define SYS_PRCNTRLR       		*(volatile ioport Uint16*)(0x1c05) // PRCR

/* not sure what these are just yet
#define SYS_GPIO_DIR0      		*(volatile ioport Uint16*)(0x1c06)  
#define SYS_GPIO_DIR1      		*(volatile ioport Uint16*)(0x1c07)
#define SYS_GPIO_DATAIN0   	*(volatile ioport Uint16*)(0x1c08)
#define SYS_GPIO_DATAIN1   	*(volatile ioport Uint16*)(0x1c09)
#define SYS_GPIO_DATAOUT0  	*(volatile ioport Uint16*)(0x1c0a)
#define SYS_GPIO_DATAOUT1  	*(volatile ioport Uint16*)(0x1c0b)
#define SYS_OUTDRSTR       		*(volatile ioport Uint16*)(0x1c16)
#define SYS_SPPDIR         			*(volatile ioport Uint16*)(0x1c17)
*/

