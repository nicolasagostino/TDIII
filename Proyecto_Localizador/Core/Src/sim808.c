/*
 * sim808.c
 *
 *  Created on: Sep 16, 2023
 *      Author: nico1
 */

#include "defines.h"

//***************** Variables **************************//
extern UART_HandleTypeDef huart2;
extern uint8_t cadena[3];
uint8_t Estado_Recepcion;
uint8_t comando_a_recibir=0;
uint8_t comando_recibido=0;
uint8_t estado_envio_SMS=0;
uint8_t mensaje_a_enviar=0;
uint8_t mensaje_enviandose=0;
bool primer_mensaje_enviado=false;
uint32_t contador_comando=0;
uint8_t dato_Rx[SIZE_RX];
volatile uint8_t indice =0;
//******************************************************//


//********************************************************************************
// Función:				  Recepcion_Modem
//
// Descripción:	Recepción de datos del Modem por UART
//
//********************************************************************************
void Recepcion_Modem (uint8_t dato)
{
	dato_Rx[indice++] = dato;

	if(indice >= SIZE_RX)
		indice=0;
	switch(comando_a_recibir)
	{

		case COMANDO_OK:
			switch(Estado_Recepcion)
			{
				case 0:
				default:
					if(dato=='O')
					{
						Estado_Recepcion=1;
					}
					break;

				case 1:
					if(dato=='K')
					{
						Estado_Recepcion=0;
						comando_recibido=COMANDO_OK;//Llegó el comando esperado
					}
					break;
			}
			break;

		case COMANDO_PICO:
			if(dato=='>')
			{
				Estado_Recepcion=0;
				comando_recibido=COMANDO_PICO;//Llegó el comando esperado
			}
			break;

		default:
			//Puede llegarme un comando por SMS
			switch(Estado_Recepcion)
			{
				case 0:
				default:
					if(dato=='@')
						Estado_Recepcion++;
					break;

				case 1:
					if(dato=='U')
						Estado_Recepcion++;
					break;

				case 2:
					if(dato=='B')
						Estado_Recepcion++;
					break;

				case 3:
					if(dato=='I')
					{
						Estado_Recepcion=0;
						Encolar_SMS(MSJ_UBICACION);
					}
					break;
			}
			break;
	}

}


//********************************************************************************
// Función:				  Borrar_Buffer_Rx
//
//********************************************************************************
void Borrar_Buffer_Rx(void)
{
	for(int i=0; i<SIZE_RX; i++)
	{
		dato_Rx[i]=0;
	}

	indice=0;
}

//********************************************************************************
// Función:				  Encolar_SMS
//
// Descripción:	Mete en la cola de SMS el proximo mensaje a enviar
//********************************************************************************
void Encolar_SMS(uint8_t mensaje)
{
	//Si el mensaje que quiero enviar, no se está enviando en esto momento, lo agrego en la cola
	if(mensaje != mensaje_enviandose)
	{
		mensaje_a_enviar = mensaje;
	}
}
//********************************************************************************
// Función:				  Enviar_SMS
//
// Descripción:	Envia los comandos al Modem para enviar un SMS fijo a un número
// 				predefinido, sin verificar las respuestas del modem
//********************************************************************************
void Enviar_SMS(void)
{
	uint8_t controlZ = 26; // 26 es el valor ASCII para CTRL+Z

	switch(estado_envio_SMS)
	{
		case 0:
			if(mensaje_a_enviar!=0)
			{
				mensaje_enviandose=mensaje_a_enviar;

				send_uart("\n\r SMS a enviar: ",UART_1);
				switch(mensaje_enviandose)
				{
					default:
						send_uart("Mensaje generado por error",UART_1);
					break;

					case MSJ_BOTON_PANICO:
						send_uart("Boton de Panico",UART_1);
						break;

					case MSJ_MOV_BRUSCO:
						send_uart("Movimiento Brusco",UART_1);
						break;

					case MSJ_UBICACION:
						send_uart("Ubicacion desconocida",UART_1);
						break;
				}
				send_uart("\n\r",UART_1);

				estado_envio_SMS++;
				mensaje_a_enviar=0;
				comando_a_recibir=0;
				comando_recibido=0;
			}
			break;

		case 1:
			if(!primer_mensaje_enviado)//Solo se envía la primera vez
			{
				if((!comando_a_recibir)||(contador_comando%TIEMPO_REINTENTO_COMANDO==0))
				{
					send_uart("AT+CFUN=1\r\n",UART_2);
					espero_comando(COMANDO_OK);
				}
				else if(comando_recibido == comando_a_recibir)
				{
					comando_a_recibir = 0;
					comando_recibido = 0;
					contador_comando = 0;
					estado_envio_SMS++;
				}
			}
			else
				estado_envio_SMS=6;
			break;

		case 2:
			if((!comando_a_recibir)||(contador_comando%TIEMPO_REINTENTO_COMANDO==0))
			{
				send_uart("AT+CNMI=2,2,0,0,0\r\n",UART_2);
				espero_comando(COMANDO_OK);
			}
			else if(comando_recibido == comando_a_recibir)
			{
				comando_a_recibir = 0;
				comando_recibido = 0;
				contador_comando = 0;
				estado_envio_SMS++;
			}
			break;

		case 3:
			if((!comando_a_recibir)||(contador_comando%TIEMPO_REINTENTO_COMANDO==0))
			{
				send_uart("AT+CMGF=1\r\n",UART_2);
				espero_comando(COMANDO_OK);
			}
			else if(comando_recibido == comando_a_recibir)
			{
				comando_a_recibir = 0;
				comando_recibido = 0;
				contador_comando = 0;
				estado_envio_SMS++;
			}
			break;

		case 4:
			if((!comando_a_recibir)||(contador_comando%TIEMPO_REINTENTO_COMANDO==0))
			{
				send_uart("AT+CGATT=1\r\n",UART_2);
				espero_comando(COMANDO_OK);
			}
			else if(comando_recibido == comando_a_recibir)
			{
				comando_a_recibir = 0;
				comando_recibido = 0;
				contador_comando = 0;
				estado_envio_SMS++;
			}
			break;

		case 5:
				if((!comando_a_recibir)||(contador_comando%TIEMPO_REINTENTO_COMANDO==0))
				{
					send_uart("AT+CSTT=\"igprs.claro.com.ar\"\r\n",UART_2);
					espero_comando(COMANDO_OK);
				}
				else if(comando_recibido == comando_a_recibir)
				{
					comando_a_recibir = 0;
					comando_recibido = 0;
					contador_comando = 0;
					estado_envio_SMS++;
				}
			break;

		case 6:
			if((!comando_a_recibir)||(contador_comando%TIEMPO_REINTENTO_COMANDO==0))
			{
				send_uart("AT+CSCA=\"+543200000001\"\r\n",UART_2);
				espero_comando(COMANDO_OK);
			}
			else if(comando_recibido == comando_a_recibir)
			{
				comando_a_recibir = 0;
				comando_recibido = 0;
				contador_comando = 0;
				estado_envio_SMS++;
			}
			break;

		case 7:
			if((!comando_a_recibir)||(contador_comando%TIEMPO_REINTENTO_COMANDO==0))
			{
				send_uart("AT+CMGS=\"+5491164881307\"\r\n",UART_2);
				espero_comando(COMANDO_PICO);
			}
			else if(comando_recibido == comando_a_recibir)
			{
				comando_a_recibir = 0;
				comando_recibido = 0;
				contador_comando = 0;
				estado_envio_SMS++;
			}
			break;

		case 8:
			if((!comando_a_recibir)||(contador_comando%TIEMPO_REINTENTO_COMANDO==0))
			{
				switch(mensaje_enviandose)
				{
					default:
						send_uart("Mensaje generado por error",UART_2);
					break;

					case MSJ_BOTON_PANICO:
						send_uart("Boton de Panico",UART_2);
						break;

					case MSJ_MOV_BRUSCO:
						send_uart("Movimiento Brusco",UART_2);
						break;

					case MSJ_UBICACION:
						send_uart("Ubicacion desconocida",UART_2);
						break;
				}
				HAL_UART_Transmit(&huart2, &controlZ, 1, 2000);
				espero_comando(COMANDO_OK);
			}
			else if(comando_recibido == comando_a_recibir)
			{
				comando_a_recibir = 0;
				comando_recibido = 0;
				contador_comando = 0;
				estado_envio_SMS++;
			}
			break;

		case 9:
			//Ya se tuvo que haber enviado el mensaje anterior
			mensaje_enviandose=0;
			estado_envio_SMS=0;
			primer_mensaje_enviado=true;
			send_uart("\r\n SMS ENVIADO \r\n",UART_1);
			break;

	}
}
//********************************************************************************
// Función:				  espero_comando
//
// Descripción:	Defino el comando que espero recibir del modem como respuesta
//				al que envié
//********************************************************************************
void espero_comando(uint8_t comando)
{
	comando_a_recibir = comando;

	Borrar_Buffer_Rx();

}

//********************************************************************************
// Función:				  Cancelar_SMS
//
// Descripción:	Cancela el envio de SMS
//********************************************************************************
void Cancelar_SMS(void)
{
	comando_a_recibir = 0;
	comando_recibido = 0;
	contador_comando = 0;
	mensaje_enviandose=0;
	estado_envio_SMS=0;
	send_uart("\r\n SMS CANCELADO POR TIMEOUT \r\n",UART_1);
}
