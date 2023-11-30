/*
 * mpu6050.c
 *
 *  Created on: Nov 26, 2021
 *      Author: grkm
 */

#include "mpu6050.h"

I2C_HandleTypeDef hi2c2;

uint8_t whoAreYou;
uint8_t MemData;

int16_t RAWgyroX;
int16_t RAWgyroY;
int16_t RAWgyroZ;

int16_t RAWaccelX;
int16_t RAWaccelY;
int16_t RAWaccelZ;

float Ax, Ay, Az;
float Gx, Gy, Gz;

//***************************************************************************
// Función: mpu6050Init()
//
// Descripción: Inicializa el módulo MPU6050, leyendo su identificación para
//				verificar la conexión.
//***************************************************************************
void mpu6050Init(void){

	HAL_I2C_Mem_Read(
				&hi2c2,
				mpu6050addr,
				whoAmIReg,
				1,
				&whoAreYou,
				1,
				100
				);
}

//***************************************************************************
// Función: mpu6050powerOn()
//
// Descripción: Enciende el MPU6050 configurando el registro de
//				administración de energía.
//***************************************************************************
void mpu6050powerOn(void){
	MemData = 0x00;
	HAL_I2C_Mem_Write(
			&hi2c2,
			mpu6050addr,
			powerManagmentReg,
			1,
			&MemData,
			1,
			100
			);
}

//***************************************************************************
// Función: mpu6050Sampling()
//
// Descripción: Configura la tasa de muestreo del MPU6050 para la lectura de
//				datos.
//***************************************************************************
void mpu6050Sampling(void){
	MemData = 0x07;
	HAL_I2C_Mem_Write(
			&hi2c2,
			mpu6050addr,
			sampleRateDiv,
			1,
			&MemData,
			1,
			100
			);
}

//***************************************************************************
// Función: mpu6050GyroScale(gyroScale_t scale)
//
// Descripción: Configura la escala del giroscopio del MPU6050 según la
//				escala especificada.
//***************************************************************************
void mpu6050GyroScale(gyroScale_t scale){
	MemData = 0x00 | (scale << 3);

	HAL_I2C_Mem_Write(
			&hi2c2,
			mpu6050addr,
			gyroConf,
			1,
			&MemData,
			1,
			100
			);
}

//***************************************************************************
// Función: mpu6050AccelScale(accelScale_t scale)
//
// Descripción: Configura la escala del acelerómetro del MPU6050 según la
//				escala especificada.
//***************************************************************************
void mpu6050AccelScale(accelScale_t scale){
	MemData = 0x00 | (scale << 3);

	HAL_I2C_Mem_Write(
			&hi2c2,
			mpu6050addr,
			accelConf,
			1,
			&MemData,
			1,
			100
			);
}

//***************************************************************************
// Función: mpu6050Config()
//
// Descripción: Configura el MPU6050 con parámetros predefinidos si la
//				identificación del dispositivo es la esperada (mpu6050)
//**************************************************************************
void mpu6050Config(void){
	// is valid Condition true 0x68
	mpu6050Init();

	if(validCondition1){
	// power on
		mpu6050powerOn();
	// sampling data ratio
		mpu6050Sampling();
	// gyro scale   (RAW)
		mpu6050GyroScale(degS250);
	// accel scale  (RAW)
		mpu6050AccelScale(g2);
	}
}

//***************************************************************************
// Función: mpu6050GyroRead()
//
// Descripción: Lee y calcula las velocidades angulares del giroscopio.
//***************************************************************************
void mpu6050GyroRead(void){
	uint8_t gyroData[6];
	HAL_I2C_Mem_Read(
			&hi2c2,
			mpu6050addr,
			gyroMeasure,
			1,
			gyroData,
			6,
			100
			);

	RAWgyroX = (uint16_t) (gyroData[0] << 8 | gyroData[1]);
	RAWgyroY = (uint16_t) (gyroData[2] << 8 | gyroData[3]);
	RAWgyroZ = (uint16_t) (gyroData[4] << 8 | gyroData[5]);

	Gx = RAWgyroX/131.0;
	Gy = RAWgyroY/131.0;
	Gz = RAWgyroZ/131.0;
}

//***************************************************************************
// Función: mpu6050AccelRead()
//
// Descripción: Lee y calcula las aceleraciones del acelerómetro.
//***************************************************************************
void mpu6050AccelRead(void){
	uint8_t accelData[6];
	HAL_I2C_Mem_Read(
			&hi2c2,
			mpu6050addr,
			accelMeasure,
			1,
			accelData,
			6,
			100
			);

	RAWaccelX = (uint16_t) (accelData[0] << 8 | accelData[1]);
	RAWaccelY = (uint16_t) (accelData[2] << 8 | accelData[3]);
	RAWaccelZ = (uint16_t) (accelData[4] << 8 | accelData[5]);

	Ax = RAWaccelX/16384.0;
	Ay = RAWaccelY/16384.0;
	Az = RAWaccelZ/16384.0;
}

