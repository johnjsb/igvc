#include "linesensor.h"
#include "i2c.h"
#include "uart.h"
#include <stdlib.h>
#include <stdarg.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <driverlib/i2c.h>
#include "i2c.h"
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <inc/hw_i2c.h>

static unsigned short i2c_address;

void InitializeLineSensor()
{
	I2CInit();
	i2c_address = 0x48 << 1;	
}

int ReadLineSensorArray(unsigned char *data)
{
	int i;
	int err; 
	char cmd = 0x84; //1 CH# 01 XX for request conversion. e.g 1 000 01 00 is for channel 2

	for(i=0; i<8; i++)
	{
		I2CSend(i2c_address,1,cmd); 
		err = I2CMasterErr(I2C0_MASTER_BASE);
		if(err != 0)
		{ Printf("Error:%d in i2c send",err); }
		I2CRecieve(i2c_address,&data[i],1);
		err = I2CMasterErr(I2C0_MASTER_BASE);
		if(err != 0)
		{ Printf("Error:%d in i2c recieve",err); }
		cmd += 0x10; 
	} 

	return 1;
}
