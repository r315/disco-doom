#ifndef _lis302_h_
#define _lis302_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "input.h"

/************************************
LIS302D Pinout (Top View)
        __SCK___
 Vdd_io|1  14 13|SDI
    GND|2     12|SDO
    RSV|3     11|RSV
    GND|4     10|GND           Z  
    GND|5      9|INT2       Y\ | /x
    Vdd|6  7   8|INT1         \|/
       |___CS___|
**************************************/	   

#define LIS302_READ          0x80     // only in SPI
#define LIS302_WRITE         0x00
#define LIS302_READ_MU       0x40     // select multiple data, only in SPI
#define LIS302_WHO_AM_I      0x0F
#define LIS302_ID            0x3B
#define LIS302_CTRL1         0x20
#define LIS302_CTRL2         0x21
#define LIS302_CTRL3         0x22
#define LIS302_STATUS        0x27
#define LIS302_OUT_X         0x29
#define LIS302_OUT_Y         0x2B
#define LIS302_OUT_Z         0x2D

// Status Bits
#define LIS302_STATUS_ZYXOR  0x80
#define LIS302_STATUS_ZOR    0x40
#define LIS302_STATUS_YOR    0x20
#define LIS302_STATUS_XOR    0x10
#define LIS302_STATUS_ZYXDA  0x08
#define LIS302_STATUS_ZDA    0x04
#define LIS302_STATUS_YDA    0x02
#define LIS302_STATUS_XDA    0x01

#define LIS302_ADDRESS       0x3A   //8-bit address

/*
typedef struct {
  double q; //process noise covariance
  double r; //measurement noise covariance
  double x; //value
  double p; //estimation error covariance
  double k; //kalman gain
} Kalman_Filter;
*/

/*
http://interactive-matter.eu/blog/2009/12/18/filtering-sensor-data-with-a-kalman-filter/ 
extern Kalman_Filter accel[3];
Kalman_Filter filterInit(double q, double r, double p, double intial_value);
void filterUpdate(Kalman_Filter* state, double measurement);
void getFilteredAxis(signed char *buf);
*/
typedef struct{
	int16_t x;
	int16_t y;
	int16_t z;
}Accel_Type;

void lis302_Init(void *param);
uint16_t lis302_ReadID(void);
uint16_t lis302_read(uint8_t *dst, uint16_t size);
uint16_t lis302_write(uint8_t *src, uint16_t size);

extern input_drv_t input_drv_lis302;

#ifdef __cplusplus
}
#endif

#endif
