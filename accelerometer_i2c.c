/*
	MPU9250 
	I2C max communication speed is 400kHz
	I2C address (7bit) 1101000 (AD0=0V) or 1101001 (AD0=VCC)
		eighth bit is the read/write 
	
*/


//just need to copy and paste this to the AIC sample code and have this be the only
//function in the main running... 
//It should print out the address at which the c5515 receives and ACK from a slave 
//device.
Int16 I2C_ADDRESS_FIND( Uint16 i2c_addr )

void main (void){
/*	
	To find the accelerometer (slave) address by detecting a ACK from the slave 
	7-bit address, 8th bit=0 for write command
	
*/
	int i;
	unsigned char address, SLV_ADD;
	char data;
	address = 0; //address goes up to 0x7F
	ICIMR &= 0X7D; //to turn off I2C NACK interrupt
	for(i=0; i<=125; i++){
		address += 1;
		if (address == I2C_ADDRESS_FIND( address )){
			SLV_ADD = address;
			printf("add = %x.... ADD = %x\n", address, SLV_ADD);
		}
	}
	return 0;
}

Int16 I2C_ADDRESS_FIND( Uint16 i2c_addr )
{
    /*Int16 timeout, i;*/

		I2C_IER = 0x0000;
		I2C_IER &= 0x7D; //disabling NACK interrupt.. no purpose here
        I2C_CNT = len;                    // Set length
        I2C_SAR = i2c_addr;               // Set I2C slave address
        I2C_MDR = MDR_STT                 // Set for Master Write
                  | MDR_TRX
                  | MDR_MST
                  | MDR_IRS
                  | MDR_FREE;
		//MAYBE INCREASE THE DELAY
        USBSTK5515_wait(100);              // Short delay
		if (!(I2C_STR & 0x02) == 1){ //it's a ACK, 
									//if the condition does not work, remove ! and replace 1 with 0
									// still does not work, only leave !(I2C_STR & 0x02) remaining
			return address;
		}
/*        for ( i = 0 ; i < len ; i++ )
        {
           I2C_DXR = data[i];            // Write
            timeout = 0x510;             // I2C_timeout = 1ms;
            USBSTK5515_GPIO_setOutput( 17, 1);
            do
            {
                if ( timeout-- < 0  )
                {
                	USBSTK5515_GPIO_setOutput( 17, 0);
                    USBSTK5515_I2C_reset( );
                    return -1;
                }
            } while ( ( I2C_STR & STR_XRDY ) == 0 );// Wait for Tx Ready
        }
*/
        
		else {
			I2C_MDR |= MDR_STP;             // Generate STOP
			USBSTK5515_waitusec(1000);
			return 0;
		}
}
