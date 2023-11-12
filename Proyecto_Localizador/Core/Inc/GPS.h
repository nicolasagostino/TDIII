/*
 * GPS.h
 *
 *  Created on: Oct 11, 2023
 *      Author: nico1
 */

#ifndef INC_GPS_H_
#define INC_GPS_H_


struct GPS_Data
{
	int8_t Hora;		//Signado por si se le resta el GMT
	uint8_t Minutos;
	uint8_t Segundos;

	char 	Estado;

	uint8_t	Latitud_Grados;
	float  	Latitud_Minutos;
	char  	Latitud_Direccion;

	uint8_t	Longitud_Grados;
	float  	Longitud_Minutos;
	char  	Longitud_Direccion;

	float	Velocidad;
	float	Curso;

	int8_t Dia;
	int8_t Mes;
	int8_t Anio;

	uint8_t Checksum;
};

#define SIZE_GPS 30

enum Trama_RMC
{
	ENCABEZADO,
	HORA,
	ESTADO,
	LATITUD,
	LONGITUD,
	VELOCIDAD,
	CURSO,
	FECHA,
	FIN_DE_TRAMA,
	CHECKSUM
};

#define REINICIAR_TRAMA	255

#define GMT	-3 //Argentina

#define GOOGLE_MAPS	 0
#define GOOGLE_EARTH 1

void Recepcion_GPS (uint8_t);
void Espero_Fin_Estado (uint8_t);
void Espero_Punto(uint8_t);
void ajustarSegunGMT(struct GPS_Data *dt, int GMT_offset);
void Armar_Ubi_Google(uint8_t, uint8_t, _Bool);

#endif /* INC_GPS_H_ */
