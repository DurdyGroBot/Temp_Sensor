
#include "Temp_Sensor.h"
//initializing SSI0 for GPIO port A
//PA2 = SSI0Clk
//PA3 = SSI0Fss
//PA4 = SSI0Rx
//PA5 = SSI0Tx
//These pins correspond with 0011 1100 = 0x3C
//See page 965 of tm4c datasheet for configuration information
void SPI_Init(void){
	SYSCTL_RCGCSSI_R |= 0x01;		//Enables SSI modules (See page 346 of datasheet) 0 - 3 by writing to bits 0 - 3. 0 -> disables SSI module 1 -> enables SSI module
	SYSCTL_RCGCGPIO_R |= 0x01;		//Enables GPIO_PORTA Clock -> A - F corresponds with 0 - 5, 1 -> turns clock on, 0 -> turns clock off
	while((SYSCTL_RCGCGPIO_R & 0x01) == 0){};		//Waits for clock to stabilize
	GPIO_PORTA_AFSEL_R |= 0x3C;		//SSI0 has special considerations when initializing AFSEL does not need to be enabled SEE PAGE 540
	GPIO_PORTA_PCTL_R |= 0x00220200;		//Enables alternate function for pins PA2, PA3, PA4, PA5 See page 688 and Table 23-5 on page 1351
	GPIO_PORTA_PCTL_R &= ~0x00002000;
	GPIO_PORTA_DEN_R |= 0x3C;		//Digitally enable PA2, PA3, PA4, PA5
	GPIO_PORTA_DIR_R |= 0x08;
	GPIO_PORTA_DATA_R |= 0x08;
	
	SSI0_CR1_R &= ~0x02;		//Disable the SSI -> Clear the SSE bit before configuring the rest of this register
	SSI0_CR1_R &= ~0xFFFFFFFF;		//For master operations, set this register to 0x00000000
	SSI0_CC_R	&= ~0x0F;		//Uses System Clock
	SSI0_CPSR_R = 0x05;		//Clock Prescale value SSInClk = SysClk / (CPSDVSR * (1+CSR)) -> CSR = 0 CPSDVSR = 2 SysClk = 50e6 ------> SSInClk = 10Mhz (max value)
	SSI0_CR0_R |= 0x0F;		//CSR = 0 SPO = SPH = 0 Freescale SPI Frame Format 16 bit data transfer
	SSI0_CR1_R |= 0x02;		//Enable SSI	
}

void write_SPI(uint16_t reg_addr){
	GPIO_PORTA_DATA_R &= ~0x08;
	while((SSI0_SR_R&0x02) == 0);
	SSI0_DR_R = reg_addr;
}

uint16_t read_SPI(uint16_t reg_addr){
	GPIO_PORTA_DATA_R &= ~0x08;
	uint16_t recieve;
	while((SSI0_SR_R & 0x02) == 0){};		//SSI transmit FIFO not full
	SSI0_DR_R = reg_addr;
	while((SSI0_SR_R & 0x04) == 0){};		//SSI recieve FIFO not empty
	recieve = SSI0_DR_R;
	return recieve;
}
