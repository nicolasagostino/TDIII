/*
 * mpu6050.h
 *
 *  Created on: Nov 26, 2021
 *      Author: grkm
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "stm32f1xx_hal.h"

//***************** Defines **************************//
#define mpu6050           0x68
#define mpu6050addr       mpu6050 << 1
#define whoAmIReg         0x75 // to read 0x68 is exist or not
#define powerManagmentReg 0x6B
#define sampleRateDiv 	  0x19 // sampleRate = gyroRate / (1 + sampleDiv)
#define gyroConf		  0x1B
#define accelConf		  0x1C

#define accelMeasure      0x3B
#define gyroMeasure       0x43

#define VALOR_GIRO_BRUSCO 249	//Cuanto más chico, más sensible
#define TIEMPO_MOV_BRUSCO 10 	//Espera esta cantidad de segundos para detectar otra vez

#define validCondition1 (whoAreYou == mpu6050)

typedef enum{
	degS250  = 0,
	degS500  = 1,
	degS1000 = 2,
	degS2000 = 3
}gyroScale_t;

typedef enum{
	g2  = 0,
	g4  = 1,
	g8  = 2,
	g16 = 3
}accelScale_t;


//****************** Funciones ***********************//
void mpu6050Config(void);
void mpu6050Init(void);
void mpu6050powerOn(void);
void mpu6050Sampling(void);
void mpu6050GyroScale(gyroScale_t scale);
void mpu6050AccelScale(accelScale_t scale);

void mpu6050GyroRead(void);
void mpu6050AccelRead(void);
//****************************************************//



#endif /* INC_MPU6050_H_ */
