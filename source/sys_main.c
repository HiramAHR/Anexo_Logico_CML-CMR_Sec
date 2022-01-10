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


struct sInputs                 //Estructura para organizar las entradas del micro controlador
{
   uint32_t s10v;              //sentido de marcha
   uint32_t s11v;              //sentido de marcha
   uint32_t squelch_av;        //llega de tarjeta RF,
   uint32_t squelch_ar;        //llega de tarjeta RF,
   uint32_t bfvm ;             //llega de tarjeta RF, VM baja frecuencia que indica velocidad maxima
   uint32_t bf48;              //llega de tarjeta RF,baja frecuencia 4.8KHz velocidad maxima(15Km/h) en talleres (no implementada)
   uint32_t retro1;            //llega de tarjeta conmutadora 23khz, indica si se estan generando 23khz en CML-CMR, no implementado por el momento VERIFICAR TARJETA COMUTADORA 23KHz
   uint32_t retro2;            //llega de tarjeta conmutadora 23khz, indica si se estan generando 23khz en PA-CMC, no implementado por el momento VERIFICAR TARJETA COMUTADORA 23KHz


   uint32_t s15vssd;           //llega de tarjeta conmutadora 23khz, indica si la senal de 23khz se genera de manera correcta
   uint32_t s102K_LT;          //linea de tren acondicionada, indica alimentacion(72v)
   uint32_t s69k_LT;           //linea de tren acondicionada, indica modo de conduccion 0 para CML-CMR y 1 para PA-CMC, palanca selectora en cabina de tren
   uint32_t s58k_LT;           //linea de tren acondicionada, selecciona el modo de conduccion 0 para CML y 1 para CMR, tambien llamada DR, DN o llave DR
   uint32_t s42QVA_LT;         //linea de tren acondicionada, indica si el tren esta dentro de un unbral de velocidad "lento" por lo general para arrancar
   uint32_t s49QVA_LT;         //linea de tren acondicionada, indica si el tren esta dentro de un otro unbral de velocidad "lento", no implementado por el momento VERIFICAR LAMPARA VLR
   uint32_t QVA1;              //senal generada mediante hardware(and a transistor) indica si la velocidad es adecuada para arrancar

};


struct sOutputs                 //Estructura para organizar las salidas del micro controlador
{                               //Las toda las senales de salida van a la tarjeta de seguridad dinamica
    uint32_t ref_15kmh_bf;      //Referencia velocidad 15 km/h, antes conocidos como punto A15
    uint32_t ref_25kmh_hf;      //Referencia velocidad 25 km/h, antes conocidos como punto A25
    uint32_t ref_60kmh_vm;      //Referencia velocidad 60 km/h, antes conocidos como punto A60
    uint32_t ref_35kmh_dr;      //Referencia velocidad 35 km/h, antes conocidos como punto A35
    uint32_t s15vssd_rebucleo;  //indica si la senal de 23khz se genera de manera correcta, se usa para validar que todas las tarjetas estan funcionando (Conmutadora23khz -> Anexo logico -> Seguridad Dinamica)
    uint32_t EC;                //EC o energia cable, indica si las condiciones para el desfrenado en CML o CMR se cumplen
    uint32_t CP;                //CP "mando perdido" se genera apartir de las senales QVA1 y MonoCoup(76LT, estado de las puertas),se trasferira su generacion a Seguridad dinamica
};




void ActualizarEntradas(void);
void ActualizarSalidas(void);
void Automantenimiento(void);
void ConsolaSerial(void);
void Texto( uint8 * nombre ,uint8 estado);
void logicaVel(void);

struct sInputs entradas;
struct sOutputs salidas;
uint32_t automantenimiento=0;

/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */

    uint32_t delay=0;
    muxInit();  //Inicializa el periferico pinmux
    gioInit();  //Inicializa el periferico gio
    sciInit();   // iniciaizar el modulo sci en el modo UART
    mibspiInit();
    hetInit();

    while(1)
    {

        ActualizarEntradas();

        Automantenimiento();

        logicaVel();

        ActualizarSalidas();

        if(delay > 50000)
        {
            ConsolaSerial();
            delay=0;
        }
        else
        {
            delay++;
        }

    }

/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
void logicaVel(void)
{
    uint32_t ref25=0,ref60=0,ref35=0,ref15=0,EC=0,aux=0;

    ref25 = (entradas.s10v||entradas.s11v) && ( !(entradas.bfvm ));

    ref60 = entradas.bfvm && ( !(entradas.s58k_LT ));

    ref35 = entradas.s58k_LT;

    ref15 = (automantenimiento || entradas.bf48) && (!(entradas.squelch_av));

    aux = (automantenimiento || entradas.s10v || entradas.s11v || entradas.bf48);

    EC = aux || (  (!(aux)) && entradas.s58k_LT );

    salidas.ref_15kmh_bf = ref15;
    salidas.ref_25kmh_hf = ref25;
    salidas.ref_35kmh_dr = ref35;
    salidas.ref_60kmh_vm = ref60;
    salidas.EC = EC;
    salidas.CP = 0; // no se puede generar este valor dentro de anexo logico, se generara dentro de seguridad dinamica
    salidas.s15vssd_rebucleo = entradas.s15vssd;
}

void Automantenimiento(void)
{
    //uint32_t flag=0;
    //flag=entradas.s10v||entradas.s11v;
    //flag=(automantenimiento)||flag;
    //flag= entradas.squelch_ar && flag;
    //automantenimiento = flag;

    automantenimiento = (entradas.s10v||entradas.s11v||automantenimiento) && entradas.squelch_ar;

}

void ActualizarSalidas(void)
{
    gioSetBit(hetPORT1,PIN_HET_9,salidas.ref_15kmh_bf);
    gioSetBit(hetPORT1,PIN_HET_4,salidas.ref_25kmh_hf);
    gioSetBit(hetPORT1,PIN_HET_26,salidas.ref_35kmh_dr);
    gioSetBit(hetPORT1,PIN_HET_20,salidas.ref_60kmh_vm);
    gioSetBit(hetPORT1,PIN_HET_8,salidas.s15vssd_rebucleo);
    gioSetBit(hetPORT1,PIN_HET_28,salidas.EC);
    gioSetBit(hetPORT1,PIN_HET_24,salidas.CP);
}

void ActualizarEntradas(void)
{
    entradas.s10v = gioGetBit(hetPORT1, PIN_HET_10);
    entradas.s11v = gioGetBit(hetPORT1, PIN_HET_12);
    entradas.squelch_av = gioGetBit(hetPORT1, PIN_HET_14);
    entradas.squelch_ar = gioGetBit(hetPORT1, PIN_HET_30);
    entradas.bfvm = gioGetBit(hetPORT1, PIN_HET_16);
    entradas.bf48 = gioGetBit(hetPORT1, PIN_HET_18);
    entradas.retro1 = gioGetBit(hetPORT1, PIN_HET_11);
    entradas.retro2 = gioGetBit(mibspiPORT5, PIN_CS0);
    entradas.s15vssd = gioGetBit(hetPORT1, PIN_HET_7);
    entradas.s102K_LT = gioGetBit(gioPORTA, 5);
    entradas.s69k_LT = gioGetBit(gioPORTA, 6);
    entradas.s58k_LT = gioGetBit(gioPORTA, 7);
    entradas.s42QVA_LT =gioGetBit(gioPORTA, 2);
    entradas.s49QVA_LT = gioGetBit(hetPORT1, PIN_HET_22);
    entradas.QVA1 = gioGetBit(mibspiPORT3, PIN_CS1);
}

void ConsolaSerial(void)
{
    sciSend(sciREG, strlen("---- ENTRADAS ---- \r\n"),"---- ENTRADAS ---- \r\n");
    Texto("s10v_v.......",entradas.s10v);
    Texto("s11v_v       ",entradas.s11v);
    Texto("squelch_av_v.",entradas.squelch_av);
    Texto("squelch_ar_v ",entradas.squelch_ar);
    Texto("bfvm_v.......",entradas.bfvm);
    Texto("bf48_v       ",entradas.bf48);
    Texto("retro1_v.....",entradas.retro1);
    Texto("retro2_v     ",entradas.retro2);

    Texto("s15vssd_v....",entradas.s15vssd);
    Texto("s102K(72v)   ",entradas.s102K_LT);
    Texto("s69k.........",entradas.s69k_LT);
    Texto("s58k(DR)     ",entradas.s58k_LT);
    Texto("s42QVA.......",entradas.s42QVA_LT);
    Texto("s49QVA       ",entradas.s49QVA_LT);
    Texto("QVA1.........",entradas.QVA1);

    sciSend(sciREG, strlen("---- SALIDAS ---- \r\n"),"---- SALIDAS ---- \r\n");
    Texto("ref_15kmh_bf ",salidas.ref_15kmh_bf);
    Texto("ref_25kmh_hf.",salidas.ref_25kmh_hf);
    Texto("ref_35kmh_dr ",salidas.ref_35kmh_dr);
    Texto("ref_60kmh_VM.",salidas.ref_60kmh_vm);
    Texto("s15v_rebucleo",salidas.s15vssd_rebucleo);
    Texto("EC...........",salidas.EC);
    Texto("CP           ",salidas.CP);

    //sciSend(sciREG, strlen("\r\n\r\n"),"\r\n\r\n");
}

void Texto( uint8 * nombre ,uint8 estado)
{
    uint8 * activo = ".ON\r\n";
    uint8 * noActivo="OFF\r\n";
    sciSend(sciREG, strlen(nombre),nombre);
    if(estado)
    {
        sciSend(sciREG, strlen(activo),activo);
    }
    else
    {
        sciSend(sciREG, strlen(noActivo),noActivo);
    }
}

/* USER CODE END */
