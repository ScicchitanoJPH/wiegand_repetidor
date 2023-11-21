# Control de Acceso Automatizado con ESP32 y Wiegand

Este código en C tiene como objetivo automatizar el acceso de dos empresas distintas que utilizan tarjetas RFID anunciadas a través de un mismo lector Wiegand. Dependiendo de la empresa a la que pertenezca la tarjeta, el sistema dirigirá al usuario a través de la puerta correspondiente. El proceso se realiza de la siguiente manera:

1. **Lectura y Decodificación:**
   - Un lector Wiegand está configurado para escuchar datos de tarjetas RFID en los pines GPIO especificados.
   - El sistema identifica el formato de la tarjeta, ya que cada empresa tiene un formato de tarjeta distinto.

2. **Selección de Puerta:**
   - Si la tarjeta pertenece a la Empresa A, el sistema retransmite el dato Wiegand al Puerto 1.
   - Si la tarjeta pertenece a la Empresa B, el sistema retransmite el dato Wiegand al Puerto 2.

## Configuración

- **Configuración del Lector:**
  - Establece los pines GPIO para el lector Wiegand (`CONFIG_EXAMPLE_D0_GPIO`, `CONFIG_EXAMPLE_D1_GPIO`).
  - Define los formatos de tarjeta admitidos por cada empresa.

- **Configuración de Puertos:**
  - Configura los pines GPIO para los Puertos Wiegand 1 y 2 (`WD1_ENCODER_D0_GPIO`, `WD1_ENCODER_D1_GPIO`, `WD2_ENCODER_D0_GPIO`, `WD2_ENCODER_D1_GPIO`).
  - Asocia cada formato de tarjeta a un puerto Wiegand.

## Uso

1. Flashea el código en tu dispositivo ESP32.
2. Conecta el lector Wiegand y configura los pines GPIO según la empresa.
3. Conecta los Puertos Wiegand a las puertas correspondientes.
4. Monitorea la salida de la consola para la identificación y retransmisión de datos Wiegand.

## Notas

- El código utiliza tareas y colas FreeRTOS para procesamiento asíncrono.
- La función `procesarValor` selecciona el puerto Wiegand adecuado según el formato de la tarjeta y retransmite los datos a la puerta correspondiente.

Siéntete libre de adaptar el código según los formatos de tarjeta y la configuración específica de tu sistema.
