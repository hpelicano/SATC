#pragma section Includes
#pragma page "Includes"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <cextdecs>


#include "satpro.h"
#include "dll.h"
#include <logutil.h>
#include <process.h>
#include <zsysc>

#define extptr

#include<xncext>


#pragma section procesar_mensaje
#pragma page "procesar_mensaje"
/*============================================================================*/
/* @Method: procesar_mensaje(char* sNetSysName)                               */
/* @Brief:  Funcion Principal que atiende todo tipo de mensajeria entrante    */
/*          al modulo Satellite.                                              */
/*          El mismo puede recibir mensajes del Nucleo, Comandos de NCPCOM    */
/*          y mensajes de otros procesos , etc                                */
/* @Params: [char* sNetSysName] - mensaje entrande (Comando / Msg)            */
/* @Return: [OK = 1] / [Abend: en caso de falla ]                             */
/*============================================================================*/
short procesar_mensaje(char* sNetSysName)
{
    short rcvType = 0;
    short srcType;
    short n1,n2,n3,n4;
    char* rcvMessage;
    short rcvMessageLen;
    short failedMessage;
    short fail_type = 0;
    short main_loop = true;
    

    char rcvBuffer[4096];
    char srcName[16 + 1];

    memset(rcvBuffer,0x00,sizeof(rcvBuffer));
    memset(srcName,0x00,sizeof(srcName)-1);

    memcpy(srcName,sNetSysName,sizeof(srcName));

    while(main_loop)
    {
        /* recibo un mensaje y tomo su origen */

        rcvType = PROCESS_EsperarMensaje(rcvBuffer, -1);

        /* * POR TIPO DE MENSAJE * */
        switch(rcvType)
        {
            case -1:
                    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                    "SATC - LOOP DE MANSAJES CANCELADO POR ERROR EN XPNET");
                    main_loop = false;
                    break;

            case RCV_TIMED_OUT_L:
                    /* Time out en AWAITIO */
                    /* time-out a la espera de mensajes */
                    break;

            case NETLIB_NET_SYSMSG_L:
                    /* Mensaje del sistema */
                    break;

            case NETLIB_NET_STOPMSG_L:
                    /* Mensaje de stop desde el NUCLEO */
                    LOGUTIL_Log(LLPANIC,MSG_FREE_FORM,"MENSAJE DE STOP DESDE EL NUCLEO");
                    main_loop = false;
                    break;

            case NETLIB_NONNET_SYSMSG_L:
                    /*Received a system message from a process directly.  One that*/
                    /*is not initiated by the network itself. For example, a proce*/
                    /*performs a guardian open (void) of my ppd name without netwo*/
                    /*intervention.  This procedure will be called if at netlib_in*/
                    /*time we specified that we wanted to get system messages.    */
                    break;

            case NETLIB_NONNET_DATAMSG_L:
                    /* A non network related message was received.  For example,  */
                    /* a process performed a writeread to my process directly.    */
                    /* This would be prior to a system open (nonnetwork).         */
                    break;

            case NETLIB_NET_DATAMSG_L:
                    /* Mensaje desde el NUCLEO */
                    /* Obtengo el puntero al mensaje y su tamanio */
                    rcvMessage = &rcvBuffer[MHDR_GET_MSG_TEXT_OFFSET(rcvBuffer)];
                    rcvMessageLen = MHDR_GET_MSG_LENGTH(rcvBuffer);
                    rcvMessage[rcvMessageLen] = '\0';

                    MHDR_GET_MSG_PROCESS_WORDS(rcvBuffer, &n1, &n2, &n3, &n4);
                    failedMessage = MHDR_GET_MSG_FAILURE(rcvBuffer, &fail_type);

                    /* Obtengo el origen del mensaje */
                    MHDR_GET_SYMBOLIC_SRC(rcvBuffer, srcName);
                    srcType = MHDR_GET_SRC_TYPE(rcvBuffer);

                    /* * Por tipo del origen del mensaje * */
                    switch(srcType)
                    {
                        case MHDR_SYM_TYPE_INTERNAL_L:
                                /* Mensajes de NCS */
                                /* Por aca me llegan los delivers de NCS */
                                ProcesarComandos(rcvMessage, rcvMessageLen);
                                break;

                        case MHDR_SYM_TYPE_PROCESS_L:
                                /* mensajes desde otro proceso */
                                procesar_mensaje_red(rcvMessage, rcvMessageLen, srcName);
                                break;

                        default:
                                break;
                    }

            default:
                    break;

        }

    }/*while*/


    return 1;
}


#pragma section procesar_mensaje_red
#pragma page "procesar_mensaje_red"
/*============================================================================*/
/* @Method: procesar_mensaje_red(char* pstm, char* p_originator)              */
/* @Brief: Aqui se procesaran los mensajes recibidos desde otros procesos     */
/*         del ecosistema de Base24 y se derivaran a las funciones de libreria*/
/*         dinamica, segun corresponda el transaction type "0200"-"0210", etc */
/*         El ruteo se correspondera con el nombre de la funcion, ej:         */
/*         --> DLLProcesarRequest()                                           */
/*         --> DLLProcesarResponse()                                          */
/*         --> DLLInfo()                                                      */
/*         --> DLLClose()                                                     */
/* @Params: pstm_def* pstm - punter a estructura de tipo PSTM                 */
/* @Return: [OK = 0] / [Error: para cualquier otro valor ]                    */
/*============================================================================*/
short procesar_mensaje_red(char* rcvMessage, short rcvMessageLen, char* srcName){
short rc = 0 ;
short number_file;
short msg_len;
char pstm_date[6];
char destino[16] = "";

tkn_area = NULL;
pstm = NULL;

        /* ---------------------------------------------- */
        /* Casteo el mensaje entrante a tipo 'pstm_def'   */
        /* ---------------------------------------------- */
        pstm = ( pstm_def* )rcvMessage;

        /* ---------------------------------------------- */
        /* Valido si es Producto ATM que maneja el STM    */
        /* ---------------------------------------------- */
        if ( memcmp( pstm->prod_id, "02", 2 ) != 0 ){
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"Producto ATM desactivado.");
            rc = ORIGEN_ERROR;
        }


        /* ---------------------------------------------- */
        /* Verifico el Area de Tokens                     */
        /* ---------------------------------------------- */
        tkn_area = tkn_area_get(rcvMessage, 
                                pstm_tkn_ofst( pstm ), 
                                rcvMessageLen,
                                binary_frmt_l,
                                pstm_userdata( pstm ), 
                                &rc );
        if ( (rc != tkn_compl_ok_l) && (rc != tkn_not_exist_l) ) {
             
             LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
             "[procesar_mensaje_red]: Error al obtener el Area de Tokens.");
             return 1;
        }


        /* ------------------------------------------------ */
        /* Realizo el Routing de los Request / Response Msg */
        /* ------------------------------------------------ */
        if (!memcmp(pstm->typ, "0200", 4) ){
            rc = ProcesarRequest(pstm, count_dlls_mcb, tkn_area);
        }else if (!memcmp(pstm->typ, "0210", 4) ){
            rc = ProcesarResponse(pstm, count_dlls_mcb, tkn_area);
        }


        /* ---------------------------------------------- */
        /* Grabo los Msg en el archivo diario             */
        /* ---------------------------------------------- */
        strncpy(&pstm_date[0], pstm->tran_dat.yy, 2 );
        strncpy(&pstm_date[2], pstm->tran_dat.mm, 2 );
        strncpy(&pstm_date[4], pstm->tran_dat.dd, 2 );

        /* Comparo la fecha de la tx con la fecha de la estructura 
           global para saber en que archivo grabar la tx */
        if (!strncmp(gSTF->fecha_ant, pstm_date, 6)){
                number_file = 0;                
        }else 
        if(!strncmp(gSTF->fecha_act, pstm_date, 6)){
                number_file = 1;
        }else 
        if (!strncmp(gSTF->fecha_sig, pstm_date, 6)){
                number_file = 2;
        }else{
             /* Si por algun error no se puede definir la fecha, escribo
                en el STF de la fecha corriente para no perder el registro */
                number_file = 1; 

                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "[procesar_mensaje_red]: No se pudo definir STF. Default Actual.");
        }
        


        /* ------------------------------------------------ */
        /* En este Switch() configuro el nombre del proceso */
        /* destino al que necesito enviar la mensajeria     */
        /* ------------------------------------------------ */
        switch (rc) 
        {
            case ORIGEN: 
            case RTAU: 
            case ORIGEN_ERROR:
                /* por el momento solo dialoga con el RTAU.
                   La respuesta siempre es enviada al proceso que ha realizado
                   la llamada y se guarda en 'srcName'. */
                strncpy(destino, srcName, 16);
                break;
            default:
                break;
        }

        /* 
           NOTA:
                La idea es pasarle el PSTM / Token y el largo total 
                para que pueda escribir en el archivo 
         */

        /* calculo las longitudes totales  */
        msg_len = calc_int_intrn_msg_lgth( pstm, tkn_area );

        /* Escribo el archivo de transacciones STF */
        rc = Write_Msg_to_STF(number_file, pstm, msg_len);
        if (rc){ /* Error - dropeo el msg al loguer */
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,  "%s", pstm);
        }



        /* ----------------------------------- */
        /* Envio el mensaje al proceso Destino */
        /* ----------------------------------- */
        PROCESS_EnviarMensaje(destino, 
                             (char*) pstm, 
                              msg_len,
                              NETLIB_RCV_REPLY_L, 
                              false, 
                              0);


    return FALSE;
 
}




#pragma section Write_Msg_to_STF
#pragma page "Write_Msg_to_STF"
/*============================================================================*/
/* @Method: Write_Msg_to_STF()                                                    */
/* @Brief:  Escribo los archivos de transacciones.                            */
/* @Params: [none]                                                            */
/* @Return: [OK = 0] / [Error = 1]                                            */
/*============================================================================*/
short Write_Msg_to_STF(short number, pstm_def* p_pstm, short largo){

short f_error;
short count_written;

_cc_status cc;  


  switch (number){
    case 0:
          cc = WRITEX ( gSTF->fd_ant, (char*)p_pstm, largo, &count_written);
          if(cc != 0){
            FILE_GETINFO_ (gSTF->fd_ant, &f_error);

            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
              "[write_Msg_to_STF]: Error [ %d ] al escribir registro STF", f_error);
            return 1;
          }
          break;

    case 1:
          cc = WRITEX ( gSTF->fd_act, (char*)p_pstm, 1174, &count_written);
          if(cc != 0){
            FILE_GETINFO_ (gSTF->fd_act, &f_error);

            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
              "[write_Msg_to_STF]: Error [ %d ] al escribir registro STF", f_error);
            return 1;
          }
          break;

    case 2:
          cc = WRITEX ( gSTF->fd_sig, (char*)p_pstm, largo, &count_written);
          if(cc != 0){
            FILE_GETINFO_ (gSTF->fd_sig, &f_error);

            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
              "[write_Msg_to_STF]: Error [ %d ] al escribir registro STF", f_error);
            return 1;
          }
          break;


    default:
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, "Verificar el archivo STF." );
      return TRUE; /* ERROR !!! */
  }



  return FALSE; /*OK*/
}