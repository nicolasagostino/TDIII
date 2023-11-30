/*
 * GPS.c
 *
 *  Created on: Oct 11, 2023
 *      Author: nico1
 */

#include "defines.h"

#ifdef USE_GPS
uint8_t indice_gps=0;
uint8_t Estado_Recepcion_GPS=0;
struct GPS_Data GPS;
struct GPS_Data GPS_aux;
uint8_t pos_gps;
uint8_t checksum_aux=0;
_Bool flag_primer_GPS=false;
//********************************************************************************
// Función:				  Recepcion_GPS
//
// Descripción:	Recepción de datos del módulo GPS (Trama GPRMC)
//
//********************************************************************************
void Recepcion_GPS (uint8_t dato)
{

	pos_gps++;
	if(pos_gps==100)
		pos_gps=0;

	if((dato != '*')&&(Estado_Recepcion_GPS!=CHECKSUM))
		GPS_aux.Checksum = GPS_aux.Checksum ^ dato; //XOR

	switch(Estado_Recepcion_GPS)
	{
		case REINICIAR_TRAMA:
		default:
			Estado_Recepcion_GPS=ENCABEZADO;
			indice_gps=0;
			pos_gps=0;
			break;

		case ENCABEZADO:
			//$GPRMC
			switch(indice_gps)
			{
				default:
				case 0:
					GPS_aux.Checksum = 0;
					//Espero el $ para empezar
					if(dato=='$')
						indice_gps++;
					break;

				case 1:
					if(dato=='G')
						indice_gps++;
					else
						Estado_Recepcion_GPS=REINICIAR_TRAMA;
					break;

				case 2:
					if(dato=='P')
						indice_gps++;
					else
						Estado_Recepcion_GPS=REINICIAR_TRAMA;
					break;

				case 3:
					if(dato=='R')
						indice_gps++;
					else
						Estado_Recepcion_GPS=REINICIAR_TRAMA;
					break;

				case 4:
					if(dato=='M')
						indice_gps++;
					else
						Estado_Recepcion_GPS=REINICIAR_TRAMA;
					break;

				case 5:
					if(dato=='C')
						indice_gps++;
					else
						Estado_Recepcion_GPS=REINICIAR_TRAMA;
					break;

				case 6:
					Espero_Fin_Estado(dato);
					break;
			}
			break;

		case HORA:
			switch(indice_gps)
			{
				default:
					Estado_Recepcion_GPS=REINICIAR_TRAMA;
				break;

				case 0:
					GPS_aux.Hora=(dato-'0')*10;
					indice_gps++;
					break;

				case 1:
					GPS_aux.Hora=GPS_aux.Hora+(dato-'0');
					indice_gps++;
					break;

				case 2:
					GPS_aux.Minutos=(dato-'0')*10;
					indice_gps++;
					break;

				case 3:
					GPS_aux.Minutos=GPS_aux.Minutos+(dato-'0');
					indice_gps++;
					break;

				case 4:
					GPS_aux.Segundos=(dato-'0')*10;
					indice_gps++;
					break;

				case 5:
					GPS_aux.Segundos=GPS_aux.Segundos+(dato-'0');
					indice_gps++;
					break;

				case 6:
					Espero_Punto(dato);
					break;

				case 7:
					//Me quedo esperando la coma porque no se cuando llega
					if(dato==',')
					{
						Estado_Recepcion_GPS++;
						indice_gps=0;
					}
					break;

			}
			break;

		case ESTADO:
			if(!indice_gps)
			{
				GPS_aux.Estado = dato;
				indice_gps++;
			}
			else
			{
				//(indice_gps==1)
				Espero_Fin_Estado(dato);
			}
			break;

		case LATITUD:
			switch(indice_gps)
			{
				default:
					Estado_Recepcion_GPS=REINICIAR_TRAMA;
				break;

				case 0:
					GPS_aux.Latitud_Grados=(dato-'0')*10;
					indice_gps++;
					break;

				case 1:
					GPS_aux.Latitud_Grados=GPS_aux.Latitud_Grados+(dato-'0');
					indice_gps++;
					break;

				case 2:
					GPS_aux.Latitud_Minutos=(dato-'0')*10;
					indice_gps++;
					break;

				case 3:
					GPS_aux.Latitud_Minutos=GPS_aux.Latitud_Minutos+(dato-'0');
					indice_gps++;
					break;

				case 4:
					Espero_Punto(dato);
					break;

				case 5:
					GPS_aux.Latitud_Minutos=GPS_aux.Latitud_Minutos+(((float)dato-'0')*0.1);
					indice_gps++;
					break;

				case 6:
					GPS_aux.Latitud_Minutos=GPS_aux.Latitud_Minutos+(((float)dato-'0')*0.01);
					indice_gps++;
					break;

				case 7:
					GPS_aux.Latitud_Minutos=GPS_aux.Latitud_Minutos+(((float)dato-'0')*0.001);
					indice_gps++;
					break;

				case 8:
					GPS_aux.Latitud_Minutos=GPS_aux.Latitud_Minutos+(((float)dato-'0')*0.0001);
					indice_gps++;
					break;

				case 9:
					GPS_aux.Latitud_Minutos=GPS_aux.Latitud_Minutos+(((float)dato-'0')*0.00001);
					indice_gps++;
					break;

				case 10:
					GPS_aux.Latitud_Minutos=GPS_aux.Latitud_Minutos+(((float)dato-'0')*0.000001);
					indice_gps++;
					break;

				case 11:
					//Si no me llega una coma, algo anda mal...
					if(dato==',')
						indice_gps++;
					else
						Estado_Recepcion_GPS=REINICIAR_TRAMA;
					break;

				case 12:
					GPS_aux.Latitud_Direccion = dato;
					indice_gps++;
					break;

				case 13:
					Espero_Fin_Estado(dato);
					break;

			}
			break;

		case LONGITUD:
			switch(indice_gps)
			{
				default:
					Estado_Recepcion_GPS=REINICIAR_TRAMA;
				break;

				case 0:
					GPS_aux.Longitud_Grados=(dato-'0')*100;
					indice_gps++;
					break;

				case 1:
					GPS_aux.Longitud_Grados=GPS_aux.Longitud_Grados+((dato-'0')*10);
					indice_gps++;
					break;

				case 2:
					GPS_aux.Longitud_Grados=GPS_aux.Longitud_Grados+(dato-'0');
					indice_gps++;
					break;

				case 3:
					GPS_aux.Longitud_Minutos=(dato-'0')*10;
					indice_gps++;
					break;

				case 4:
					GPS_aux.Longitud_Minutos=GPS_aux.Longitud_Minutos+(dato-'0');
					indice_gps++;
					break;

				case 5:
					Espero_Punto(dato);
					break;

				case 6:
					GPS_aux.Longitud_Minutos=GPS_aux.Longitud_Minutos+(((float)dato-'0')*0.1);
					indice_gps++;
					break;

				case 7:
					GPS_aux.Longitud_Minutos=GPS_aux.Longitud_Minutos+(((float)dato-'0')*0.01);
					indice_gps++;
					break;

				case 8:
					GPS_aux.Longitud_Minutos=GPS_aux.Longitud_Minutos+(((float)dato-'0')*0.001);
					indice_gps++;
					break;

				case 9:
					GPS_aux.Longitud_Minutos=GPS_aux.Longitud_Minutos+(((float)dato-'0')*0.0001);
					indice_gps++;
					break;

				case 10:
					GPS_aux.Longitud_Minutos=GPS_aux.Longitud_Minutos+(((float)dato-'0')*0.00001);
					indice_gps++;
					break;

				case 11:
					GPS_aux.Longitud_Minutos=GPS_aux.Longitud_Minutos+(((float)dato-'0')*0.000001);
					indice_gps++;
					break;

				case 12:
					//Si no me llega una coma, algo anda mal...
					if(dato==',')
						indice_gps++;
					else
						Estado_Recepcion_GPS=REINICIAR_TRAMA;
					break;

				case 13:
					GPS_aux.Longitud_Direccion = dato;
					indice_gps++;
					break;

				case 14:
					Espero_Fin_Estado(dato);
					break;
			}
			break;

		case VELOCIDAD:
			switch(indice_gps)
			{
				default:
					Estado_Recepcion_GPS=REINICIAR_TRAMA;
				break;

				case 0:
					GPS_aux.Velocidad = (dato-'0');
					indice_gps++;
					break;

				case 1:
					if(dato == '.')
						indice_gps++; //Si viene un punto, significa que ya terminó la parte entera
					else
						GPS_aux.Velocidad = (GPS_aux.Velocidad * 10) + (dato-'0'); //Sino lo sumo, como un nuevo dígito
					break;

				case 2:
					GPS_aux.Velocidad=GPS_aux.Velocidad+(((float)dato-'0')*0.1);
					indice_gps++;
					break;

				case 3:
					GPS_aux.Velocidad=GPS_aux.Velocidad+(((float)dato-'0')*0.01);
					indice_gps++;
					break;

				case 4:
					Espero_Fin_Estado(dato);
					break;
			}
			break;

		case CURSO:
			switch(indice_gps)
			{
				default:
					Estado_Recepcion_GPS=REINICIAR_TRAMA;
				break;

				case 0:
					GPS_aux.Curso = (dato-'0');
					indice_gps++;
					break;

				case 1:
					if(dato == '.')
						indice_gps++; //Si viene un punto, significa que ya terminó la parte entera
					else
						GPS_aux.Curso = (GPS_aux.Curso * 10) + (dato-'0'); //Sino lo sumo, como un nuevo dígito
					break;

				case 2:
					GPS_aux.Curso=GPS_aux.Curso+(((float)dato-'0')*0.1);
					indice_gps++;
					break;

				case 3:
					GPS_aux.Curso=GPS_aux.Curso+(((float)dato-'0')*0.01);
					indice_gps++;
					break;

				case 4:
					Espero_Fin_Estado(dato);
					break;
			}
			break;

		case FECHA:
			switch(indice_gps)
			{
				default:
					Estado_Recepcion_GPS=REINICIAR_TRAMA;
				break;

				case 0:
					GPS_aux.Dia=(dato-'0')*10;
					indice_gps++;
					break;

				case 1:
					GPS_aux.Dia=GPS_aux.Dia+(dato-'0');
					indice_gps++;
					break;

				case 2:
					GPS_aux.Mes=(dato-'0')*10;
					indice_gps++;
					break;

				case 3:
					GPS_aux.Mes=GPS_aux.Mes+(dato-'0');
					indice_gps++;
					break;

				case 4:
					GPS_aux.Anio=(dato-'0')*10;
					indice_gps++;
					break;

				case 5:
					GPS_aux.Anio=GPS_aux.Anio+(dato-'0');
					indice_gps++;
					break;

				case 6:
					ajustarSegunGMT(&GPS_aux, GMT);
					Espero_Fin_Estado(dato);
					break;
			}
			break;

		case FIN_DE_TRAMA:
			//Me quedo esperando el *
			if(dato=='*')
				Estado_Recepcion_GPS++;
			break;

		case CHECKSUM:

			if(indice_gps==0)
			{
				if(dato>'A')
					checksum_aux= (dato-'A'+10)*16;
				else
					checksum_aux= (dato-'0')*16;
				indice_gps++;
			}
			else
			{
				if(dato>'A')
					checksum_aux= checksum_aux+(dato-'A'+10);
				else
					checksum_aux= checksum_aux+(dato-'0');

				if(GPS_aux.Checksum==checksum_aux)
				{
					if(GPS.Estado!=GPS_aux.Estado)
					{
						send_uart(" Toma GPS!!! \n\r",UART_1);
						if(!flag_primer_GPS)
							Encolar_SMS(TOMO_GPS);
						flag_primer_GPS=true;
					}

					GPS = GPS_aux;
				}

				Espero_Fin_Estado(dato);//para reiniciar todas las variables
				Estado_Recepcion_GPS=ENCABEZADO;
			}
			break;
	}

}


//********************************************************************************
// Función:				  Espero_Fin_Estado
//
// Descripción:	Funcion usada cuando en la trama debería llegar una coma
//********************************************************************************
void Espero_Fin_Estado(uint8_t espero_coma)
{
	//Si no me llega una coma, algo anda mal...
	if(espero_coma==',')
	{
		Estado_Recepcion_GPS++;
		indice_gps=0;
	}
	else
		Estado_Recepcion_GPS=REINICIAR_TRAMA;
}

//********************************************************************************
// Función:				  Espero_Punto
//
// Descripción:	Funcion usada cuando en la trama debería llegar un punto
//********************************************************************************
void Espero_Punto(uint8_t espero_punt)
{
	//Si no me llega una coma, algo anda mal...
	if(espero_punt=='.')
		indice_gps++;
	else
		Estado_Recepcion_GPS=REINICIAR_TRAMA;
}

//********************************************************************************
// Función:				  ajustarSegunGMT
//
// Descripción:	Función para ajustar la fecha y hora según GMT
//********************************************************************************
void ajustarSegunGMT(struct GPS_Data *dt, int GMT_offset)
{
    // Asegurarse de que el desplazamiento esté en el rango [-12, 12]
    if (GMT_offset < -12 || GMT_offset > 12)
    {
        //Desplazamiento GMT no válido
        return;
    }

    // Sumar el desplazamiento GMT a la hora
    dt->Hora += GMT_offset;

    // Manejar desbordamientos y ajustes en la hora
    if (dt->Hora < 0)
    {
        dt->Hora += 24;
        dt->Dia -= 1;
    }
    else if (dt->Hora >= 24)
    {
        dt->Hora -= 24;
        dt->Dia += 1;
    }

    // Ajustar el mes y el año si es necesario
    if (dt->Mes < 1)
    {
        dt->Mes = 12;
        dt->Anio -= 1;
    }
    else if (dt->Mes > 12)
    {
        dt->Mes = 1;
        dt->Anio += 1;
    }

    // Asegurarse de que los días sean válidos para el mes
    int maxDias = 31; // Máximo valor por defecto
    if (dt->Mes == 4 || dt->Mes == 6 || dt->Mes == 9 || dt->Mes == 11)
    {
        maxDias = 30;
    }
    else if (dt->Mes == 2)
    {
        if ((dt->Anio % 4 == 0 && dt->Anio % 100 != 0) || dt->Anio % 400 == 0)
        {
            maxDias = 29; // Año bisiesto
        }
        else
        {
            maxDias = 28; // Año no bisiesto
        }
    }

    if (dt->Dia < 1)
    {
        dt->Dia = maxDias;
        dt->Mes -= 1;
    }
    else if (dt->Dia > maxDias)
    {
        dt->Dia = 1;
        dt->Mes += 1;
    }
}
//********************************************************************************
// Función:				  Armar_Ubi_Google
//
// Descripción:	Ubica el punto en el que estamos ubicados en el Google Maps o
//				Google Earth
//********************************************************************************
void Armar_Ubi_Google(uint8_t uart_envio, uint8_t app, _Bool guardar_FS)
{

	char Buffer_Maps[100];
	int latitud_grados_aux;
	int longitud_grados_aux;
	int latitud_min_aux;
	int longitud_min_aux;
	float minutos_aux;

	if(GPS.Latitud_Direccion=='S')
		latitud_grados_aux=-GPS.Latitud_Grados;
	else
		latitud_grados_aux=GPS.Latitud_Grados;

	minutos_aux=GPS.Latitud_Minutos/60;
	minutos_aux=minutos_aux*10000000;
	latitud_min_aux=minutos_aux;

	if(GPS.Longitud_Direccion=='W')
		longitud_grados_aux=-GPS.Longitud_Grados;
	else
		longitud_grados_aux=GPS.Longitud_Grados;

	minutos_aux=GPS.Longitud_Minutos/60;
	minutos_aux=minutos_aux*10000000;
	longitud_min_aux=minutos_aux;

	switch(app)
	{
		default:
		case GOOGLE_MAPS:
			//Genero Link para visualizar en Google Maps
			sprintf(Buffer_Maps,"https://www.google.com/maps/@%i.%d,%i.%d,20z (%02d/%02d/%02d %02d:%02d:%02d) ",latitud_grados_aux,latitud_min_aux,longitud_grados_aux,longitud_min_aux,GPS.Dia,GPS.Mes,GPS.Anio,GPS.Hora,GPS.Minutos,GPS.Segundos);
			send_uart(Buffer_Maps,uart_envio);
			break;

		case GOOGLE_EARTH:
			//Escribo la latitud y longitud en el formato que interpreta Google Earth
			//Ej:-58.5276153,-34.6658579,0
			//send_uart("-58.5276153,-34.6658579,0\n",uart_envio);
			sprintf(Buffer_Maps,"%i.%d,%i.%d,0\n",longitud_grados_aux,longitud_min_aux,latitud_grados_aux,latitud_min_aux);
			send_uart(Buffer_Maps,uart_envio);
			break;
	}

	if(guardar_FS)
		send_uart("\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r", uart_envio);//Por si se queda corto... siempre espera la cantidad de bytes que le dije

}

#endif
