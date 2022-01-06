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

struct sPort{
    gioPORT_t * puerto;
    uint32_t numero;
    uint32_t valor;
};

struct sInputs
{
   struct sPort s10v_v;
   struct sPort s11v_v;
   struct sPort squelch_av_v;
   struct sPort squelch_ar_v;
   struct sPort bfvm_v ;
   struct sPort bf48_v;
   struct sPort retro1_v;
   struct sPort retro2_v;


   struct sPort s15vssd_v;
   struct sPort s102K;
   struct sPort s69k;
   struct sPort s58k;
   struct sPort s42QVA;
   struct sPort s49QVA;
   struct sPort QVA1;

};


struct sOutputs
{
    struct sPort ref_bf;
    struct sPort ref_hf;
    struct sPort ref_vm;
    struct sPort ref_dr;
    struct sPort s15v_rebucleo;
    struct sPort ec;
    struct sPort cp;
};


void Automantenimiento(uint32_t s10v,uint32_t s11v,uint32_t squelch_TRA, uint32_t * automantenimiento)
{
    uint32_t flag=0;
    flag=s10v||s11v;
    flag=(*automantenimiento)||flag;
    flag=squelch_TRA&&flag;
    *automantenimiento=flag;

}
void GetPin(struct sPort * puerto)
{
    puerto->valor=gioGetBit(puerto->puerto, puerto->numero);
}

void setPin(struct sPort puerto)
{
    gioSetBit(puerto.puerto, puerto.numero, puerto.valor);
}

void Texto( uint8_t * nombre ,uint8_t estado)
{
    char * activo="activo \r\n";
    char * noActivo="no Activo \r\n";
    sciSend(sciREG, strlen(nombre),nombre);
    if(estado)
        sciSend(sciREG, strlen(activo),activo);
    else
        sciSend(sciREG, strlen(noActivo),noActivo);
}

void SerialEntradas(struct sInputs * Entradas)
{

    Texto("s10v_v         ",Entradas->s10v_v.valor);
    Texto("s11v_v         ",Entradas->s11v_v.valor);
    Texto("squelch_av_v   ",Entradas->squelch_av_v.valor);
    Texto("squelch_ar_v   ",Entradas->squelch_ar_v.valor);
    Texto("bfvm_v         ",Entradas->bfvm_v.valor);
    Texto("bf48_v         ",Entradas->bf48_v.valor);
    Texto("retro1_v       ",Entradas->retro1_v.valor);

    Texto("retro2_v       ",Entradas->retro2_v.valor);
    Texto("s15vssd_v      ",Entradas->s15vssd_v.valor);
    Texto("s102K          ",Entradas->s102K.valor);
    Texto("s69k           ",Entradas->s69k.valor);
    Texto("s58k           ",Entradas->s58k.valor);
    Texto("s42QVA         ",Entradas->s42QVA.valor);
    Texto("s49QVA         ",Entradas->s49QVA.valor);
    Texto("QVA1           ",Entradas->QVA1.valor);


}

void SerialSalidas(struct sOutputs * Salidas)
{
    Texto("ref_bf (15kmh) ",Salidas->ref_bf.valor);
    Texto("ref_hf (25kmh) ",Salidas->ref_hf.valor);
    Texto("ref_dr (35kmh) ",Salidas->ref_dr.valor);
    Texto("ref_VM (60kmh) ",Salidas->ref_vm.valor);
    Texto("s15v_rebucleo  ",Salidas->s15v_rebucleo.valor);
    Texto("ec             ",Salidas->ec.valor);
    Texto("cp             ",Salidas->cp.valor);

    sciSend(sciREG, strlen("\r\n\r\n"),"\r\n\r\n");
}

void initPorts(struct sInputs * Entradas, struct sOutputs * Salidas)
{
    Entradas->s10v_v.puerto=hetPORT1;
    Entradas->s10v_v.numero=PIN_HET_10;
    Entradas->s11v_v.puerto=hetPORT1;
    Entradas->s11v_v.numero=PIN_HET_12;
    Entradas->squelch_av_v.puerto=hetPORT1;
    Entradas->squelch_av_v.numero=PIN_HET_14;
    Entradas->squelch_ar_v.puerto=hetPORT1;
    Entradas->squelch_ar_v.numero=PIN_HET_30;
    Entradas->bfvm_v.puerto=hetPORT1;
    Entradas->bfvm_v.numero=PIN_HET_16;
    Entradas->bf48_v.puerto=hetPORT1;
    Entradas->bf48_v.numero=PIN_HET_18;
    Entradas->retro1_v.puerto=hetPORT1;
    Entradas->retro1_v.numero=PIN_HET_11;
    Entradas->retro2_v.puerto=mibspiPORT5;
    Entradas->retro2_v.numero=PIN_CS0;
    Entradas->s15vssd_v.puerto=hetPORT1;
    Entradas->s15vssd_v.numero=PIN_HET_7;
    Entradas->s102K.puerto=gioPORTA;
    Entradas->s102K.numero=5;
    Entradas->s69k.puerto=gioPORTA;
    Entradas->s69k.numero=6;
    Entradas->s58k.puerto=gioPORTA;
    Entradas->s58k.numero=7;
    Entradas->s42QVA.puerto=gioPORTA;
    Entradas->s42QVA.numero=2;
    Entradas->s49QVA.puerto=hetPORT1;
    Entradas->s49QVA.numero=PIN_HET_22;
    Entradas->QVA1.puerto=mibspiPORT3;
    Entradas->QVA1.numero=PIN_CS1;

    Salidas->ref_bf.puerto=hetPORT1;
    Salidas->ref_bf.numero=PIN_HET_9;
    Salidas->ref_hf.puerto=hetPORT1;
    Salidas->ref_hf.numero=PIN_HET_4;
    Salidas->ref_dr.puerto=hetPORT1;
    Salidas->ref_dr.numero=PIN_HET_26;
    Salidas->s15v_rebucleo.puerto=hetPORT1;
    Salidas->s15v_rebucleo.numero=PIN_HET_8;
    Salidas->ec.puerto=hetPORT1;
    Salidas->ec.numero=PIN_HET_28;
    Salidas->cp.puerto=hetPORT1;
    Salidas->cp.numero=PIN_HET_24;
    Salidas->ref_vm.puerto=hetPORT1;
    Salidas->ref_vm.numero=PIN_HET_20;


}




void inputs_sense(struct sInputs * entradas);
void Outputs(struct sOutputs salidas);
uint32_t logicaVel(struct sInputs * entradas, struct sOutputs * salidas, uint32_t automantenimiento);
/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    struct sInputs entradas;
    struct sOutputs salidas;
    uint32_t automantenimiento=0;
    uint32_t i=0;

    muxInit();  //Inicializa el periferico pinmux

    gioInit();  //Inicializa el periferico gio

    sciInit();   // iniciaizar el modulo sci en el modo UART

    mibspiInit();

    hetInit();

    initPorts(&entradas, &salidas);


    while(1)
    {
        for(i=0;i<16000000;i++);
        inputs_sense(&entradas);
        SerialEntradas(&entradas);
        Automantenimiento(entradas.s10v_v.valor, entradas.s11v_v.valor, entradas.squelch_ar_v.valor, &automantenimiento);
        logicaVel(&entradas, &salidas,automantenimiento);
        SerialSalidas(&salidas);
        Outputs(salidas);


    }

/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
uint32_t logicaVel(struct sInputs * entradas, struct sOutputs * salidas, uint32_t automantenimiento)
{
    uint32_t ref25, flag2,ref60=0,ref15=0,EC=0,aux=0;
    ref25=entradas->s10v_v.valor||entradas->s11v_v.valor;
    flag2=!(entradas->bfvm_v.valor);
    ref25= ref25&&flag2;
    ref60= entradas->bfvm_v.valor&&(!(entradas->s58k.valor));

    ref15= automantenimiento||entradas->bf48_v.valor;
    ref15= ref15&&(!(entradas->squelch_av_v.valor));
    aux = automantenimiento||entradas->s10v_v.valor||entradas->s11v_v.valor||entradas->bf48_v.valor;
    flag2=(!aux)&&(entradas->s58k.valor);
    EC=(aux||flag2);
    EC = (entradas->s10v_v.valor||entradas->s11v_v.valor)&&(entradas->s58k.valor)?0:EC;
    //EC = EC && ((entradas->squelch_av_v.valor)||(entradas->squelch_ar_v.valor));
    EC = ref25?EC && ((entradas->squelch_av_v.valor)||(entradas->squelch_ar_v.valor)):EC;
    //    EC = (entradas->s10v_v.valor||entradas->s11v_v.valor)&&(entradas->s58k.valor)&&(!(entradas->squelch_av_v.valor)&&(!(entradas->squelch_ar_v.valor)))?0:EC;
    salidas->ec.valor=EC;
    salidas->ref_bf.valor=ref15;
    salidas->ref_hf.valor=ref25;
    salidas->ref_vm.valor=ref60;
    salidas->ref_dr.valor=entradas->s58k.valor;
    salidas->cp.valor=!(entradas->s69k.valor);
    salidas->s15v_rebucleo.valor=entradas->s15vssd_v.valor;


    return 0;
}

void Outputs(struct sOutputs salidas)
{
    setPin(salidas.cp);
    setPin(salidas.ec);
    setPin(salidas.ref_bf);
    setPin(salidas.ref_dr);
    setPin(salidas.ref_hf);
    setPin(salidas.ref_vm);
    setPin(salidas.s15v_rebucleo);
}

void inputs_sense(struct sInputs * entradas)
{
    GetPin(&entradas->s10v_v);
    GetPin(&entradas->s11v_v);
    GetPin(&entradas->squelch_av_v);
    GetPin(&entradas->squelch_ar_v);
    GetPin(&entradas->bfvm_v);
    GetPin(&entradas->bf48_v);
    GetPin(&entradas->retro1_v);
    GetPin(&entradas->retro2_v);
    GetPin(&entradas->s15vssd_v);
    GetPin(&entradas->s102K);
    GetPin(&entradas->s69k);
    GetPin(&entradas->s58k);
    GetPin(&entradas->s42QVA);
    GetPin(&entradas->s49QVA);
    GetPin(&entradas->QVA1);

}

/* USER CODE END */
