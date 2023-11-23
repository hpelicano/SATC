/*!==============================================================================!*/
/*! @Author:  PELICANO, HECTOR ROMAN                                             !*/
/*! @Team:    AGUSTIN METZ.                                                      !*/
/*! @Date:    11/10/2023                                                         !*/ 
/*! @Version: V.1.0.0                                                            !*/
/*! @Proyect: PROCESO SATELLITE DE XPNET CON CARGA DE DLLs DINAMICAS             !*/
/*! @Resume:  ESTE MODULO SE ENCARGAR DE REALIZAR DOS TAREAS PRINCIPALES.        !*/
/*!           (1) PROCESAMIENTO DE COMANDO DE NCPCOM PARA ABMs DE DLLs EN MEMORIA!*/
/*!           (2) PROCESAMIENTO DE MENSAJERIA ENTRE PROCESOS DE XPNET DERIVANDO  !*/
/*!               LOS MENSAJES A LAS DLLs QUE CORRESPONDAN.                      !*/
/*!==============================================================================!*/

#pragma section includes
#pragma page "includes"
/* includes del sistema */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <process.h>
#include <logutil.h>
#include "sat.h"
#include "satinit.h"
#include "satpro.h"


#pragma section principal
#pragma page "principal"
/*============================================================================*/
/* @Method: main(int argc, char** argv, char** env)                           */
/* @Brief:  Funcion principar llamada al ejecutar el comando START            */
/* @Params: [int argc, char** argv, char** env]                               */
/* @Return: codigo de retorno 0 = Ok, -1  = Error de inicio (ABEND)           */
/*============================================================================*/
int main(int argc, char** argv, char** env)
{
    int inReturn = 0;
    /*------------------------------------------------------*/
    /* apertura e inicializacion de comunicacion y archivos */
    /*------------------------------------------------------*/
    if(PROCESS_Iniciar( NETLIB_OPT_NOWAITRCV_L  +
                        ! NETLIB_OPT_WANTSYSMSG_L +
                        ! NETLIB_OPT_WANTLOGICERR_L, 1,
                        szSysName,
                        lconf_name,
                        collector_name ) != 0)
    {
        return 1; 
    }

    /*-------------------------------------------------------*/
    /*abrir el logger */
    /* Inicializo el logger con el colector alternativo como */
    /* nuevo parametro de la funcion ($0 por defecto)        */
    /*-------------------------------------------------------*/
    if(!LOGUTIL_Iniciar(collector_name, szSysName,
                        "SATLT", "SATC","$0"))
        return -2;

    /*------------------------------------------------------------------------*/
    /* Mas vale que arranque en debug y despues en algun momento leo del LCONF*/
    /*------------------------------------------------------------------------*/
    LOGUTIL_Set_Loglevel(LLDEBUG);


    /*-------------------------------------*/
    /* Inicializacion del archivo de LCONF */
    /*-------------------------------------*/
    if(inicializar_Lconf(lconf_name))
    {
        LOGUTIL_Cerrar();
        return -3;
    }

    LOGUTIL_Set_Loglevel(loglevel);
    LOGUTIL_Log(LLPANIC, INIT_OK, "SATC", NULL);

    /*---------------------------------*/
    /* Levanto las librerias dinamicas */
    /*---------------------------------*/
    inReturn = load_dinamic_dlls();
    if(inReturn != 0){
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, "STOPPED FROM: load_dinamic_dlls()");
        PROCESS_Cerrar();
    }




    /*-------------------------------------*/
    /* Crecion del Archivo STF             */
    /*-------------------------------------*/
    inReturn = init_STF();
    if(inReturn == 10){
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, "(ERROR): Falla al crear STF");
    }





    /*------------------------------------------*/
    /* Proceso la mensajeria entrante al modulo */
    /*------------------------------------------*/
    inReturn = procesar_mensaje(szSysName);
    if(inReturn){
        /* Libero el espacio en memoria para la info global de las DLLs */
        free(gSGDll);

        /* Libero la memoria del Memory Control Block de las DLLs */
        free(gDllMcb);
        
        close_STF(0); /* Archivo de tx de ayer   */
        close_STF(1); /* Archivo de tx de hoy    */
        close_STF(2); /* Archivo de tx de mañana */

        /* Libero la memoria global de los archivos STF  */
        free(gSTF); 

        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, "--> SATELLITE STOPPED");
        PROCESS_Cerrar();
    }


    /*--------------*/
    /* Success Exit */
    /*--------------*/
    return 0;
}
