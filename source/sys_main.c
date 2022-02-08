/** @file sys_main.c 
*   @brief Application main file
*   @date 11-Dec-2018
*   @version 04.07.01
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com 
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */

#include "sci.h"
#include "mibspi.h"
#include "het.h"
#include "gio.h"
#include "sys_vim.h"
#include "pinmux.h"
#include "string.h"
#include "stdlib.h"

/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */


struct sInputs                 //Estructura para organizar las entradas del micro controlador.
{
   uint32_t i10v_RF;           //llega de tarjeta RF, indica el sentido de marcha.
   uint32_t i11v_RF;           //llega de tarjeta RF, indica el sentido de marcha.
   uint32_t iSquelch_AV_RF;    //llega de tarjeta RF, es una cadena de seguridad que indica la presencia de captores delanteros and programa B2.
   uint32_t iSquelch_AR_RF;    //llega de tarjeta RF, es una cadena de seguridad que indica la presencia de captores traseros and programa B2.
   uint32_t iBfVM_RF;          //llega de tarjeta RF, VM baja frecuencia que indica velocidad maxima.
   uint32_t iBf48_RF;          //llega de tarjeta RF,baja frecuencia 4.8KHz velocidad maxima(15Km/h) en talleres (no implementada).
   uint32_t iRetro1_C23;       //llega de tarjeta conmutadora 23khz, indica si se estan generando 23khz en CML-CMR, no implementado por el momento VERIFICAR TARJETA COMUTADORA 23KHz.
   uint32_t iRetro2_C23;       //llega de tarjeta conmutadora 23khz, indica si se estan generando 23khz en PA-CMC, no implementado por el momento VERIFICAR TARJETA COMUTADORA 23KHz.


   uint32_t i15vssd_C23;   //llega de tarjeta conmutadora 23khz, indica si la senal de 23khz se genera de manera correcta.
   uint32_t i102K_LT;      //linea de tren acondicionada, indica alimentacion(72v).
   uint32_t i69k_LT;       //linea de tren acondicionada, indica modo de conduccion 0 para CML-CMR y 1 para PA-CMC, palanca selectora en cabina de tren.
   uint32_t i58k_LT;       //linea de tren acondicionada, selecciona el modo de conduccion 0 para CML y 1 para CMR, tambien llamada DR, DN o llave DR.
   uint32_t i42QVA_LT;     //linea de tren acondicionada, indica si el tren esta dentro de un unbral de velocidad "lento" por lo general para arrancar.
   uint32_t i49QVA_LT;     //linea de tren acondicionada, indica si el tren esta dentro de un otro unbral de velocidad "lento", no implementado por el momento VERIFICAR LAMPARA VLR.
   uint32_t iQVA1_HW;      //senal generada mediante hardware(and a transistor) indica si la velocidad es adecuada para arrancar.

};


struct sOutputs                    //Estructura para organizar las salidas del micro controlador.
{                                  //Las toda las senales de salida van a la tarjeta de seguridad dinamica.
    uint32_t oRef_15kmh_BF_SD;     //Referencia velocidad 15 km/h, antes conocidos como punto A15.
    uint32_t oRef_25kmh_HF_SD;     //Referencia velocidad 25 km/h, antes conocidos como punto A25.
    uint32_t oRef_35kmh_DR_SD;     //Referencia velocidad 35 km/h, antes conocidos como punto A35.
    uint32_t oRef_60kmh_VM_SD;     //Referencia velocidad 60 km/h, antes conocidos como punto A60.
    uint32_t o15vssd_Rebucleo_SD;  //indica si la senal de 23khz se genera de manera correcta, se usa para validar que todas las tarjetas estan funcionando (Conmutadora23khz -> Anexo logico -> Seguridad Dinamica).
    uint32_t oEC_SD;               //EC o energia cable, indica si las condiciones para el desfrenado en CML o CMR se cumplen.
    uint32_t oCP_SD;               //CP "mando perdido" se genera apartir de las senales QVA1 y MonoCoup(76LT, estado de las puertas),se trasferira su generacion a Seguridad dinamica.
};




void ActualizarEntradas(void);              //Prototipo de la funcion para leer las señales de entrada.
void ActualizarSalidas(void);               //Prototipo de la funcion para mandar las señales de salida.
void Automantenimiento(void);               //Prototipo de la funcion para automantener energia cable a traves de una OR 10V o 11V y una AND con Squelch AR.
void ConsolaSerial(void);                   //Prototipo de la funcion para monitorear las señales a traves de comunicacion serial.
void Texto( uint8 * nombre ,uint8 estado);  //Prototipo de la funcion para mandar los estados de las señales ON para cuando se detecta un 1 logico y OFF para cuando se detecta un 0 logico.
void logicaVel(void);                       //Prototipo de la funcion para generar energía cable a traves de la verificacion de la velocidad y su comparacion con el umbral de velocidad activo.

struct sInputs entradas;                    //Estructura para entradas de señales
struct sOutputs salidas;                    //Estructura para salidas de señales.
uint32_t automantenimiento=0;               //Variable auxiliar para activar el automantenimiento de energia cable.

/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */

    uint32_t delay=0;   //Variable de retardo para la comunicacion serial.
    muxInit();          //Inicializa el periferico pinmux.
    gioInit();          //Inicializa el periferico gio.
    sciInit();          // iniciaizar el modulo sci en el modo UART.
    mibspiInit();       //Inicializa el periferico Multi SPI.
    hetInit();          //Inicializa el periferico Het.

    while(1)
    {

        ActualizarEntradas();   //Se llama a la funcion ActualizarEntradas.

        Automantenimiento();    //Se llama a la funcion Automantenimiento.

        logicaVel();            //Se llama a la funcion logicaVel.

        ActualizarSalidas();    //Se llama a la funcion ActualizarSalidas.

        if(delay > 50000)       //Compara la cuenta del delay.
        {
            ConsolaSerial();    //Se llama a la funcion ConsolaSerial.
            delay=0;            //Se resetea la variable delay.
        }
        else
        {
            delay++;            //Se incrementa en 1 la variable delay.
        }

    }

/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
void logicaVel(void)        //Funcion para generar energía cable a traves de la verificacion de la velocidad y su comparacion con el umbral de velocidad activo.
{
    uint32_t ref25=0, ref60=0, ref35=0, ref15=0, EC=0, aux=0;

    ref25 = (entradas.i10v_RF||entradas.i11v_RF) && ( !(entradas.iBfVM_RF ));   //Ver documento Manual del Equipo Embarcado del Sistema de Pilotaje Automatico 135 kHz pag. 102 diagrama "Elaboración de los umbrales".

    ref60 = entradas.iBfVM_RF && ( !(entradas.i58k_LT ));

    ref35 = entradas.i58k_LT;

    ref15 = (automantenimiento || entradas.iBf48_RF) && (!(entradas.iSquelch_AV_RF));

    aux = (automantenimiento || entradas.i10v_RF || entradas.i11v_RF || entradas.iBf48_RF);

    EC = aux || (  (!(aux)) && entradas.i58k_LT );          //Ver documento Manual del Equipo Embarcado del Sistema de Pilotaje Automatico 135 kHz pag. 102 diagrama "Elaboración de la Energía Cable".

    salidas.oRef_15kmh_BF_SD = ref15;
    salidas.oRef_25kmh_HF_SD = ref25;
    salidas.oRef_35kmh_DR_SD = ref35;
    salidas.oRef_60kmh_VM_SD = ref60;
    salidas.oEC_SD = EC;
    salidas.oCP_SD = 0;     //No se puede generar este valor dentro de anexo logico, se generara dentro de seguridad dinamica.
    salidas.o15vssd_Rebucleo_SD = entradas.i15vssd_C23; //Esta señal solo se transfiere a la tarjeta Conmutadora de 23kHz.
}

void Automantenimiento(void)        //Funcion para automantener energia cable a traves de una OR 10V y/o 11V y una AND con Squelch AR
{
   automantenimiento = (entradas.i10v_RF||entradas.i11v_RF||automantenimiento) && entradas.iSquelch_AR_RF;

}

void ActualizarSalidas(void)        //Funcion para mandar las señales de salida
{
    gioSetBit(hetPORT1,PIN_HET_9,salidas.oRef_15kmh_BF_SD);     //HetPort1 pin9
    gioSetBit(hetPORT1,PIN_HET_4,salidas.oRef_25kmh_HF_SD);     //HetPort1 pin4
    gioSetBit(hetPORT1,PIN_HET_26,salidas.oRef_35kmh_DR_SD);    //HetPort1 pin26
    gioSetBit(hetPORT1,PIN_HET_20,salidas.oRef_60kmh_VM_SD);    //HetPort1 pin20
    gioSetBit(hetPORT1,PIN_HET_8,salidas.o15vssd_Rebucleo_SD);  //HetPort1 pin8
    gioSetBit(hetPORT1,PIN_HET_28,salidas.oEC_SD);              //HetPort1 pin28
    gioSetBit(hetPORT1,PIN_HET_24,salidas.oCP_SD);              //HetPort1 pin24
}

void ActualizarEntradas(void)       //Funcion para leer las señales de entrada
{
    entradas.i10v_RF = gioGetBit(hetPORT1, PIN_HET_10);         //HetPort1 pin10
    entradas.i11v_RF = gioGetBit(hetPORT1, PIN_HET_12);         //HetPort1 pin12
    entradas.iSquelch_AV_RF = gioGetBit(hetPORT1, PIN_HET_14);  //HetPort1 pin14
    entradas.iSquelch_AR_RF = gioGetBit(hetPORT1, PIN_HET_30);  //HetPort1 pin30
    entradas.iBfVM_RF = gioGetBit(hetPORT1, PIN_HET_16);        //HetPort1 pin16
    entradas.iBf48_RF = gioGetBit(hetPORT1, PIN_HET_18);        //HetPort1 pin18
    entradas.iRetro1_C23 = gioGetBit(hetPORT1, PIN_HET_11);     //HetPort1 pin11
    entradas.iRetro2_C23 = gioGetBit(mibspiPORT5, PIN_CS0);     //MIBSPI5  pinCS0
    entradas.i15vssd_C23 = gioGetBit(hetPORT1, PIN_HET_7);      //HetPort1 pin7
    entradas.i102K_LT = gioGetBit(gioPORTA, 5);                 //GIO PortA pin5
    entradas.i69k_LT = gioGetBit(gioPORTA, 6);                  //GIO PortA pin6
    entradas.i58k_LT = gioGetBit(gioPORTA, 7);                  //GIO PortA pin7
    entradas.i42QVA_LT =gioGetBit(gioPORTA, 2);                 //GIO PortA pin2
    entradas.i49QVA_LT = gioGetBit(hetPORT1, PIN_HET_22);       //HetPort1 pin22
    entradas.iQVA1_HW = gioGetBit(mibspiPORT3, PIN_CS1);
}

void ConsolaSerial(void)        //Funcion para monitorear las señales a traves de comunicacion serial
{
    sciSend(sciREG, strlen("---- ENTRADAS ---- \r\n"),"---- ENTRADAS ---- \r\n");
    Texto("s10v_v.......",entradas.i10v_RF);
    Texto("s11v_v       ",entradas.i11v_RF);
    Texto("squelch_av_v.",entradas.iSquelch_AV_RF);
    Texto("squelch_ar_v ",entradas.iSquelch_AR_RF);
    Texto("bfvm_v.......",entradas.iBfVM_RF);
    Texto("bf48_v       ",entradas.iBf48_RF);
    Texto("retro1_v.....",entradas.iRetro1_C23);
    Texto("retro2_v     ",entradas.iRetro2_C23);

    Texto("s15vssd_v....",entradas.i15vssd_C23);
    Texto("s102K(72v)   ",entradas.i102K_LT);
    Texto("s69k.........",entradas.i69k_LT);
    Texto("s58k(DR)     ",entradas.i58k_LT);
    Texto("s42QVA.......",entradas.i42QVA_LT);
    Texto("s49QVA       ",entradas.i49QVA_LT);
    Texto("QVA1.........",entradas.iQVA1_HW);

    sciSend(sciREG, strlen("---- SALIDAS ---- \r\n"),"---- SALIDAS ---- \r\n");
    Texto("ref_15kmh_bf ",salidas.oRef_15kmh_BF_SD);
    Texto("ref_25kmh_hf.",salidas.oRef_25kmh_HF_SD);
    Texto("ref_35kmh_dr ",salidas.oRef_35kmh_DR_SD);
    Texto("ref_60kmh_VM.",salidas.oRef_60kmh_VM_SD);
    Texto("s15v_rebucleo",salidas.o15vssd_Rebucleo_SD);
    Texto("EC...........",salidas.oEC_SD);
    Texto("CP           ",salidas.oCP_SD);

}

void Texto( uint8 * nombre ,uint8 estado)       //Funcion para mandar los estados de las señales ON para cuando se detecta un 1 logico y OFF para cuando se detecta un 0 logico
{
    char * activo = ".ON\r\n";
    char * noActivo="OFF\r\n";
    sciSend(sciREG, strlen((const char *)nombre),(uint8 *)nombre);
    if(estado)
    {
        sciSend(sciREG, strlen(activo),(uint8 *)activo);
    }
    else
    {
        sciSend(sciREG, strlen(noActivo),(uint8 *)noActivo);
    }
}

/* USER CODE END */
