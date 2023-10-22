/*
 * sim808.h
 *
 *  Created on: Sep 16, 2023
 *      Author: nico1
 */

#ifndef INC_SIM808_H_
#define INC_SIM808_H_

//***************** Defines **************************//
#define UART_1	1
#define UART_2	2


#define SIZE_RX 30

//Comandos esperados del modem
#define COMANDO_OK 	 1	//OK
#define COMANDO_PICO 2	//>


#define TIEMPO_REINTENTO_COMANDO	 20  // 200
#define MAXIMA_ESPERA_COMANDOS		 500 //5000

enum{
	MSJ_BOTON_PANICO=1,
	MSJ_MOV_BRUSCO,
	MSJ_UBICACION,
	TOMO_GPS,
};

//****************** Funciones ***********************//
void Recepcion_Modem (uint8_t);
void Borrar_Buffer_Rx(void);
void Encolar_SMS(uint8_t);
void Enviar_SMS(void);
void espero_comando(uint8_t);
void Cancelar_SMS(void);

//****************************************************//

#endif /* INC_SIM808_H_ */
