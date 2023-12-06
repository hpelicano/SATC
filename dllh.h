#ifndef _DLL_H_
#define _DLL_H_


#ifndef _BADDLC_
#define _BADDLC_
    #include <baddlc>
#endif

#include "satinit.h"
#include <token.h>


#define ORIGEN 0
#define RTAU   1
#define MDS    2
#define BNET   3
#define VISA   4 
#define ORIGEN_ERROR 9

/* Comando para Warmboot de proceso cierra los STF's y abre los nuevos*/
#define WARMBOOT "9502"

/* ejemplo de comando para Alta de Libreria Dinamica:
   [ DELIVER P <PROCESS>, "9500LOAD -name=AA10 -vol=$data26" ] 
    -Formato de Datos guardados en archivo: 
    "9500LOAD-PRSP,A,$DATA26.DLLSPRSP.DLLPRSP"        */
#define DLL_LOAD "9500LOAD"

/* ejemplo de comando para Suspender una Libreria Dinamica: 
   [ DELIVER P <PROCESS>, "9500SUSP -name=AA10" ]       */
#define DLL_SUSP "9500SUSP"

/* ejemplo de comando para Activar una Libreria Dinamica: 
   [ DELIVER P <PROCESS>, "9500ACTV -name=AA10" ]  */
#define DLL_ACTV "9500ACTV"

/* ejemplo de comando para Deletear una Libreria Dinamica: 
   [ DELIVER P <PROCESS>, "9500DELT -name=AA10" ]    */
#define DLL_DELT "9500DELT"

/* ejemplo de comando para Listar todas las librerias activas: 
   [ DELIVER P <PROCESS>, "9500LIST -name=****" ]    */
#define DLL_LIST "9510LIST"

short ProcesarRequest(pstm_def* pstm, short count_dlls_mcb,tkn_area_def* tokens);
short ProcesarResponse(pstm_def* pstm, short count_dlls_mcb,tkn_area_def* tokens);

short ProcesarComandos(char* p_msg_txtptr, short rcvMessageLen);
short abm_dlls_dinamicas(char* p_msg_txtptr, short rcvMessageLen);
short listar_dlls(char* p_msg_txtptr, short rcvMessageLen);
short warmboot_dlls();
short warmboot_STF_Files();

#endif