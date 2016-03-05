//
// Vishtasb Namiranian
// CS0
//#define SPICMD2 *(volatile Uint16 )0x3005
// need SPICMD2_READ = 0x39 (CS0, 8-bit characters, read)
// need SPICMD2_WRITE = 0x3a (CS0, 8-bit characters, write)
//
/*
 * main.c
 */
#include "stdio.h"

#define Uint16  unsigned short
//system registers
#define EBSR *(volatile ioport Uint16*)(0x1c00)
#define PCGCR1 *(volatile ioport Uint16*)(0x1c02) //DSP peripheral clock configuration
#define PSRCR *(volatile ioport Uint16*)(0x1c04)
#define PRCR *(volatile ioport Uint16*)(0x1c05)
//SPI registers
#define SPICMD2 *(volatile ioport Uint16*)(0x3005) //Chooses the CS line, char length, read/write mode
#define SPICMD1 *(volatile ioport Uint16*)(0x3004)
#define SPICDR *(volatile ioport Uint16*)(0x3000)
#define SPICCR *(volatile ioport Uint16*)(0x3001) //Clock Control Register
#define SPISTAT1 *(volatile ioport Uint16*)(0x3006) //Status register with Character and Frame count, used for checking
#define SPISTAT2 *(volatile ioport Uint16*)(0x3007) //Character count per frame transfer
#define SPIDAT1 *(volatile ioport Uint16*)(0x3008)
#define SPIDAT2 *(volatile ioport Uint16*)(0x3009)
#define SPIDCR1 *(volatile ioport Uint16*)(0x3002) //Device configuration register
#define SPIDCR2 *(volatile ioport Uint16*)(0x3003)
//SPI register values
#define SPICMD2_READ 0x39 //puts SPI in read mode with 8-bit data transfer on CS0
#define SPICMD2_WRITE 0x3a //puts SPI in write mode with 8-bit data transfer on CS0
#define SPICCR_CLKEN 0x8000 //enables the SPI clock (divides the system clock to get the SPI clk)
#define SPICLK_ONLY 0x7ffd //only enables the SPI clock (0111,1111,1111,1101)
#define SPICLK_RATE 0xffff //183Hz
#define SPIDCR1_CS0 0X5 //CS0 active low, SPI mode 3, 0 delay
#define SPIDCR2_LPB 0x8101 //loopback enabled
#define SPIDCR2_NLPB 0x101 //loopback disabled
#define SPICMD1_3CHAR 0xc002 //Frame and Character interrupts enabled with 3(2+1) characters per frame
#define SPICMD1_INTEN 0xc000 //FIRQ and CIRQ enabled with FLEN=0 (1 character per frame)

void SPI_init (void);
void FrameLength_setup (Uint16 frameLength);
void SPI_dataSend (char output_data, Uint16 characterLength);//, int characterLength);
void SPI_dataReceive (void);
void SPIDAT12_clear(void);


int main(void) {

	SPI_init();
	FrameLength_setup(3);
	
	SPI_dataSend(0x783a, 8);

	while(1);

	return 0;
}

void SPI_init (void){ //SPI register setup/initialization with focus on CS0 settings
	PSRCR = 0x008; //Peripheral Software Reset Control Register, duration that the reset signal is asserted
	PRCR = 0x80; //0000,0000,1000,0000 is to reset SPI only for 0x8 system clock cycles
	while(PRCR & 0x080){ //checks for SPI reset release, PRCR-bit7==0  
		asm(" nop");
	}
	PCGCR1 = SPICLK_ONLY; //not needed for spi, just for practice
	SPICCR = 0; //disable spi clock. or set to 0x4000 to include the spi software reset 
	SPICDR = SPICLK_RATE;
	SPICCR = SPICCR_CLKEN; //enables spi clock
	
	
	SPIDCR1 = SPIDCR1_CS0; 
/* ----may need to set EBSR too--------------------------*/
	EBSR = 0x6c3f;
	SPIDCR2 = SPIDCR2_LPB; //loopback=RX and TX registers are internally connected
}
	
void FrameLength_setup (Uint16 frameLength){
	SPICMD1 = SPICMD1_INTEN; //FIRQ and CIRQ enabled with CLEN=0
	frameLength -= 1; //subtracting 1 from frameLength since frame length=CLEN+1
	SPICMD1 &= frameLength; //updating the frame length of data transfer 
}	
	
void SPI_dataSend (char output_data, Uint16 characterLength){
	char temp;
	Uint16 i;
	SPIDAT12_clear();
	//output_data[]=01101000
	//--------------00010110
	//SPIDAT2 register must be loaded then the SPICMD2 set for write.
	for(i=0;i<characterLength;i++){
		temp = *output_data >> i; 
		temp & 1;
		SPIDAT2 |= temp;
		SPIDAT2 <<= 1;
	}
	SPICMD2 = SPICMD2_WRITE;
	
	while(!((SPISTAT1 & 1)& 1)); //checks the status of SPI line and if it has finished transferring
								//data it allows to exit the SPI_dataSend


}

void SPI_dataReceive (void){
	//must check the status of the spi bus, BUSY=0 and CC=1 is clear 
	while(!((SPISTAT1 & 3) ^ 2)& 2); //temporary use. most likely will need to consider the other reserved 
							//bits to not interfere with the correctness of the result.
	SPIDAT12_clear();
	SPICMD2 = SPICMD2_READ;



}

void SPIDAT12_clear(void){ //clears the SPI Data registers. Registers are not cleared between reads or writes.
	SPIDAT1 = 0;
	SPIDAT2 = 0;
}
