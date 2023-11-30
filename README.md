# **TDIII**
# Proyecto: Dispositivo de Localización y Seguimiento

## Se agrega el código desarrollado con STM32CubeIDE, con las siguientes configuraciones:

- USART1: Se utilizará para recibir datos del módulo GPS y para hacer el debug del dispositivo 
	- PB6: TX
	- PB7: RX

- USART2: Se utilizará para la comunicación con el módulo SIM808
	- PA2: TX
	- PA3: RX

- I2C2: Se utilizará para la comunicación con el módulo MPU6050
	- PB10: SCL
	- PB11: SDA

- PULSADOR: Se utilizará como entrada para el pulsador de pánico.
	- PA0: GPIO_Input

- LED: Se utilizará como salida para indicar los distintos estados.
	- PC13: GPIO_Output

 # PROYECTO FINALIZADO

![Image text](https://github.com/nicolasagostino/TDIII/blob/main/Documentos/Proyecto_Finalizado.jpg)
