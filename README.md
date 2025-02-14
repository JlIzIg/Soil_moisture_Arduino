This scheme is based on two Arduino devices, one of which (ARD1) is connected to a sensor, while the second (ARD2) is connected to an LCD display of the JHD-2X16-I2C type.
When the button is pressed, the following is ensured:
a. exchange of key information using the Diffie-Hellman protocol using random numbers that change in each communication session;
b. reading information from the sensor;
c. encryption of information using the AES cryptographic algorithm with the key obtained during the execution of the Diffie-Hellman protocol;
d. transmission of information using UART to the second Arduino device.
Also, when the soil moisture threshold values ​​are exceeded, the indicators show this.
