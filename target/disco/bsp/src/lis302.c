
#include "lis302.h"
#include "board.h"

static i2cbus_t *i2cbus;
/**
 * 
 */
void lis302_Init(void *param)
{
uint8_t data[2];
	//data = 0;
	//EXT_I2C_WRITE(LIS302_ADDRESS, LIS302_CTRL1, &data, 1);		//power down
	data[0] = LIS302_CTRL1;
	data[1] = 0x47;
	i2cbus = (i2cbus_t*)param;
	i2cbus->write(LIS302_ADDRESS, data, 2);		// power it up, enable axis
	//data = 0x0
	//EXT_I2C_WRITE(LIS302_ADDRESS, LIS302_CTRL2, &data, 1);		// 
	//data = 0x0
	//EXT_I2C_WRITE(LIS302_ADDRESS, LIS302_CTRL3, &data, 1);		// power it up, enable axis
}

uint16_t lis302_ReadID(void){
uint8_t data = LIS302_WHO_AM_I;
	i2cbus->write(LIS302_ADDRESS, &data, 1);
	i2cbus->read(LIS302_ADDRESS, &data, 1);	
	return data;
}      
/**
 * 
 */
uint16_t lis302_read(uint8_t *dst, uint16_t size)
{
uint8_t data;

	// Read Status
	data = LIS302_STATUS;
	i2cbus->write(LIS302_ADDRESS, &data, 1);
	i2cbus->read(LIS302_ADDRESS, &data, 1);
	
	// Check is has new data
	if( (data & LIS302_STATUS_ZYXDA) == 0 ){
		*(uint16_t*)dst = 0xFFFF;
		return 0;
	}

	// if( data & LIS302_STATUS_ZYXOR ){		
	// 	lis302_read(dst, size);
	// }

	//Read axis data
	data = LIS302_OUT_X | (1<<7);
	i2cbus->write(LIS302_ADDRESS, &data, 1);
    i2cbus->read(LIS302_ADDRESS, dst, size); 

	return size;	
}

/**
 * 
 */
uint16_t lis302_write(uint8_t *src, uint16_t size){
	return 0;
}

#if 0
//--------------------------------------
// ACCEL_AXIS(OUT_X);
//--------------------------------------
signed char getAxis(unsigned char axis)
{
signed char tmp;   
	
	for(tmp=0; tmp<50; tmp++){
		SELECT_ACCEL;	
		SPI_Send(accel_spi, LIS302_READ | LIS302_STATUS);
		if(SPI_Send(accel_spi, 0xFF) & LIS302_ZYXDA){
			DESELECT_ACCEL;
			break;
		}
		DESELECT_ACCEL;
	}	
	
	SELECT_ACCEL;	
	SPI_Send(accel_spi, ACCEL_RD|axis);
	tmp = SPI_Send(accel_spi, 0xFF);
	DESELECT_ACCEL;
	return tmp;
}


//--------------------------------------
//
//--------------------------------------
Kalman_Filter filterInit(double q, double r, double p, double intial_value)
{
  Kalman_Filter result;
  result.q = q;
  result.r = r;
  result.p = p;
  result.x = intial_value;

  return result;
}
//--------------------------------------
//
//--------------------------------------
void filterUpdate(Kalman_Filter* state, double measurement)
{
  //prediction update
  //omit x = x
  state->p = state->p + state->q;

  //measurement update
  state->k = state->p / (state->p + state->r);
  state->x = state->x + state->k * (measurement - state->x);
  state->p = (1 - state->k) * state->p;
}
//--------------------------------------
//
//--------------------------------------
void getFilteredAxis(signed char *buf)
{
unsigned char i;
	getAllaxis(buf);
	
	for(i=0;i<3;i++)
	{
		filterUpdate(&accel[i],buf[i]);	
		buf[i] = accel[i].x;		
	}
}

#endif
