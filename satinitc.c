#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <cextdecs>
#include <dlaunchh>
#include <tal.h>
#include <dlfcn.h>


#include <zsysc>

#include "satinit.h"
#include <lconf.h>
#include <logutil.h>

extern char szSysName[17]; 
extern char lconf_name[27];


#pragma section inicializar_Lconf
#pragma page "inicializar_Lconf"
/*============================================================================*/
/* @Function: inicializar_Lconf(const char* szLncfName)                       */
/* @Breaf: Esta funcion carga en memoria los PARAMS y ASSIGNS del Lconf       */
/*         para luego poder utilizar los recursos configurados.               */
/*         Dentro del primer assign conseguimos obtener el archivo de las     */
/*         librerias dinamicas a cargar: "DLLS-DINAMICAS"                     */
/*============================================================================*/
short inicializar_Lconf(const char* szLncfName)
{
  short shLconfResult = 0;
  short i;
  /* Inicializar variables globales */
  memset(gLconfAssigns, 0x00, sizeof(gLconfAssigns));
  memset(gLconfParams, 0x00, sizeof(gLconfParams));
  memset(szSysName,0x00,sizeof(szSysName));

    /* Cargo los Assigns necesarios para el proceso */
    i=0;
    gLconfAssigns[i++].assign = "DLLS-DINAMICAS";
    gLconfAssigns[i++].assign = "SAT-PTLF";
    gLconfAssigns[i].assign   = NULL ;



  if(LCONF_GetAssigns(szSysName, szLncfName, gLconfAssigns))
  {
    LOGUTIL_Log(LLPANIC, LCONF_ERROR,
                "inicializar_lconf: Error en LCONF_GetAssigns: %s", LCONF_Error());
    return 1;
  }

  LCONF_static_message_string[0] = 0x00;

  for(i = 0; gLconfAssigns[i].assign != NULL; i++)
  {
    if( !strcmp(gLconfAssigns[i].file_name, "NO DEFINIDO"))
    {
      sprintf(&LCONF_static_message_string[strlen(LCONF_static_message_string)],
              "A: %s  ", gLconfAssigns[i].assign);
      return 1;
    }
  }

    /* Cargo los Params necesarios para el proceso */
    i=0;
    gLconfParams[i].param   = "POS-DH-MAX-TERMS                ";
    gLconfParams[i++].param = "POS-DH-MAX-TERMS-IN-DYN-TBL     ";
    gLconfParams[i++].param = "POS-DH-MAX-TERMS-IN-STATIC-TBL  ";
    gLconfParams[i++].param = "POS-HPDH-MAX-HPCFS              ";
    gLconfParams[i++].param = "POS-HPDH-MAX-HPTCS              ";
    gLconfParams[i++].param = "POS-HPDH-AUTH-RESP-TIMEOUT      ";
    gLconfParams[i++].param = "POS-HPDH-OLDMSG-TIMEOUT         ";
    gLconfParams[i++].param = "POS-HPDH-LOG-TERM-STATS         ";
    gLconfParams[i++].param = "POS-HPDH-REPA-PROCESS           ";
    gLconfParams[i++].param = "POS-HPDH-DEVOLUCION-ON          ";
    gLconfParams[i++].param = "POS-HPDH-PLAN-GOBIERNO-ON       ";
    gLconfParams[i++].param = "POS-HPDH-PLAN-GOBIERNO-DIAS     ";
    gLconfParams[i++].param = "POS-HPDH-IMPACTA-MONTO-EMV      ";
    gLconfParams[i++].param = "POS-HPDH-INY                    ";
    gLconfParams[i++].param = "POS-HPDH-LOG-ON                 ";
    gLconfParams[i++].param = "POS-DH-DUKPT-UPDATE-METHOD      ";
    gLconfParams[i++].param = "POS-DH-KSN-DESCR                ";
    gLconfParams[i++].param = "POS-DH-KSN-MAX-DIFFERENCE       ";
    gLconfParams[i++].param = "POS-DH-KSN-APPRV-OPT            ";
    gLconfParams[i++].param = "POS-DH-KSN-CNTR-APPRV-OPT       ";
    gLconfParams[i++].param = "POS-CLOVER-RANGE-TERMS          ";
    gLconfParams[i].param   = NULL ;


  if(LCONF_GetParams(szSysName, szLncfName, gLconfParams) != 0)
  {
     LOGUTIL_Log(LLPANIC, LCONF_ERROR,
                  "inicializar_lconf: Error en LCONF_GetParams: %s", LCONF_Error());
    return 2;
  }

  LCONF_static_message_string[0] = 0x00;

  for(i = 0; gLconfParams[i].param != NULL; i++)
  {
    if( !strcmp(gLconfParams[i].param_txt, "NO DEFINIDO"))
    {
      sprintf(&LCONF_static_message_string[strlen(LCONF_static_message_string)],
              "P: %s  ", gLconfParams[i].param);

      shLconfResult = 2;
    }
  }

  switch (shLconfResult)
  {
    case 0:     /* todo Ok */
          break;
    case 1:     /* falta algun assign */
          LOGUTIL_Log(LLPANIC, LCONF_ERROR,
                   "inicializar_lconf: Falta algun assign en el LCONF: %s", LCONF_Error());
          break;
    case 2:     /* falta algun param */
          LOGUTIL_Log(LLPANIC, LCONF_ERROR,
                   "inicializar_lconf: Falta algun param en el LCONF: %s", LCONF_Error());
          break;
    default:    /* fruta */
          LOGUTIL_Log(LLPANIC, LCONF_ERROR,
                   "inicializar: Error al leer el LCONF: %s", LCONF_Error());
          shLconfResult = -1;
      break;
  }

  loglevel = (short)atoi(gLconfParams[0].param_txt);

  return shLconfResult;
}




#pragma section load_dinamic_dlls
#pragma page "load_dinamic_dlls"
/*============================================================================*/
/* @Method: load_dinamic_dlls()                                               */
/* @Brief:  Esta funcion se encarga de armar una estructura de memoria con un */
/*          mapeo de las direcciones de memoria de las funciones de librerias */
/*          dinamicas para poder enviar y recibir mensajes de otros procesos  */
/* @Params: [none]                                                            */
/* @Return: [OK = 0] / [Error = 1]                                            */
/*============================================================================*/
short load_dinamic_dlls(){

_cc_status cc;
const char assign[14] = "DLLS-DINAMICAS";

short rc;
short fd_modulos;
short file_eof   = false;
short encontrado = false;
short error = false;
short i;
short largo;
short inicio;
short path_len = 0;
short indice   = 0;
unsigned short read_count = 31;
unsigned short count_read;

char file_modulos[36];
char file_name[36];
char record_buffer[33];
char msg_loguer[43];
char msg_err[80];

  /*---------------------------------*/
  /*(0) Inicializo a un puntero nulo */
  /*---------------------------------*/
  gSGDll  = NULL;  
  gDllMcb = NULL;
  count_dlls_mcb = 0;

  /*-------------------------------------------------------------------------*/
  /*(1) busco el assign para obtener el path junto con el archivo de modulos */
  /*-------------------------------------------------------------------------*/
  i = 0;
  while (!encontrado){

    if (!memcmp(assign, gLconfAssigns[i].assign, sizeof(assign))){
    
      memcpy(file_modulos, gLconfAssigns[i].file_name, sizeof(file_modulos));
      encontrado = true;
    
    }else{
      /*Incremento el indice*/
      i = i + 1;
    }
  }

  /*-------------------------------------------------------------------------*/
  /*(2) Obtengo la longitud exacta del path del archivo                      */
  /*-------------------------------------------------------------------------*/
  for (i=0; file_modulos[i] != NULL ; i++){
    if(file_modulos[i] == '$'){ 
      inicio = i; 
    }
  }
  
  largo = (i - inicio);
  strncpy(file_name, &file_modulos[inicio], largo);


  /*-------------------------------*/
  /*(3) Abro el archivo de modulos */
  /*-------------------------------*/  
  if (encontrado){

    rc = FILE_OPEN_(file_name, largo, &fd_modulos);
    if (rc != 0){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                      "No se pudo Abrir el archivo de Modulos Dinamicos.");
      return 1;
    }

  }

  /*------------------------------------------------------------------------*/
  /*(4) Inicializo estrutura global y Guardo la info del archivo de MODULOS */
  /*------------------------------------------------------------------------*/  
  gSGDll = (SGDLL*)malloc(sizeof(SGDLL));
  if (gSGDll == NULL){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Fallo la asignacion de memoria inicial para gSGDll");
    return 1;
  }

  /* Inicializo con ceros la memoria asignada */
  memset(gSGDll, 0x00, sizeof(SGDLL));

  /* guardo el dato */
  gSGDll->path_name_len = largo;
  memcpy(gSGDll->path_name, file_name, largo);

  memset(record_buffer,0x00,sizeof(record_buffer));


  /*guardo el nombre del Proceso y el LCONF*/
  memcpy(gSGDll->szSysName, szSysName, sizeof(szSysName)); 
  memcpy(gSGDll->lconf_name, lconf_name, sizeof(lconf_name));



  /*------------------------------------------------------------------------*/
  /*(5) Cuento los registros que hay en el archivo MODULOS                  */
  /*------------------------------------------------------------------------*/  
  while (!file_eof) {
    cc = READX( fd_modulos, record_buffer, read_count, &count_read );
    if(cc > 0 ){
      file_eof = true;
    }else{
      count_dlls_mcb = count_dlls_mcb + 1;
    }  
  }

  /* Si no hay registros aviso en el loguer y retorno. */
  if (count_dlls_mcb == 0 ){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "No hay registros de DLLs configurados en el archivo 'MODULOS'");
    
    /*Cierro el archivo de MODULOS, ya tengo cargado en memoria los registros*/
    FILE_CLOSE_ (fd_modulos);
    
    /*No hago que abendee para que funcione para procesar comandos*/
    return false;
  }

  /*------------------------------------------------------------------------*/
  /*(6) Dimensiono la memoria para almacenar todos los registros            */
  /*------------------------------------------------------------------------*/
  gDllMcb = (DLL_MCB*)malloc(count_dlls_mcb * sizeof(DLL_MCB));
  if (gDllMcb == NULL){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Fallo la asignacion de memoria inicial para gDllMcb");
    return 1;
  }

  /*------------------------------------------------------------------------*/
  /*(7) Inicializo con ceros la memoria asignada                            */
  /*------------------------------------------------------------------------*/
  memset(gDllMcb, 0x00, (count_dlls_mcb * sizeof(DLL_MCB)));




  /*------------------------------------------------------------------------*/
  /*(8) Me reposiciono al inicio del archivo de MODULOS                     */
  /*------------------------------------------------------------------------*/
  KEYPOSITION ( fd_modulos,0,,0 );

  /*------------------------------------------------------------------------*/
  /*(9) Comienzo a cargar los datos en la Memoria                           */
  /*------------------------------------------------------------------------*/
  memset(record_buffer,0x00,sizeof(record_buffer));
  for(i=0; i < count_dlls_mcb; i++){

    cc = READX( fd_modulos, record_buffer, read_count, &count_read );
    if(cc > 0 ){
      break;
    }

    strncpy(gDllMcb[i].prsp  , &record_buffer[0],  4);
    strncpy(gDllMcb[i].status, &record_buffer[5],  1);

    /* "PRSP,A,$DATA26.DLLSPRSP.DLLPRSP" */
    if(!strncmp(&record_buffer[7], "$", 1)){
      indice = 7;
      while( strncmp(&record_buffer[indice],"\0", 1) ){
          path_len = path_len + 1;
          indice = indice + 1;
      } 
    }else{
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, 
        "(Error) Fallo la lectura del Path .");
        return 1;
    }

    strncpy(gDllMcb[i].path_file, &record_buffer[7], path_len);

  }


  /*--------------------------------------------------------------------------*/
  /*(10) Para finalizar, completo la estructura global en memoria: 'DLL_MCB' 
         usando las APIs de librerias dinamicas para obtener las direcciones
         de memoria de cada una de las funciones.                             */
  /*--------------------------------------------------------------------------*/
  for (i=0; i < count_dlls_mcb; i++){

    /*[A]- Obtengo la dlHandle de la libreria */
    gDllMcb[i].dllHandle = dlopen(gDllMcb[i].path_file, 
                                  RTLD_NOLOAD | RTLD_VERBOSE(2));

    if(gDllMcb[i].dllHandle == 0){
      gDllMcb[i].dllHandle = dlopen(gDllMcb[i].path_file, 
                                  RTLD_LAZY | RTLD_VERBOSE(2));
    }


    if (!gDllMcb[i].dllHandle){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                      "Error al obtener el Handle de las Dlls. [%s]", 
                      gDllMcb[i].path_file);
      return 1;
    }

    /* Si se carga bien el Handle de la libreria, dejo registro en el loguer */
    memset(msg_loguer,' ', sizeof(msg_loguer));
    strcpy(msg_loguer, "Cargando DLL Memory Control Block : [");
    strncat(msg_loguer, gDllMcb[i].path_file, 24 );
    strncat(msg_loguer, "]", 1 );
    LOGUTIL_Log(LLINFO, MSG_FREE_FORM, msg_loguer);

    /*[B]- Obtengo el Address de la DLLInfo */
    gDllMcb[i].dllInfo = (short (*)(char*))dlsym(gDllMcb[i].dllHandle, "DLLInfo");
    if (!gDllMcb[i].dllInfo){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"Error al obtener Address de DLLInfo");
      error = true;
    }

    /*[C]- Obtengo el Address de la DLLProcesarRequest */
    gDllMcb[i].dllProcesarRequest = 
          (short (*)(pstm_def*, tkn_area_def*, LCONF_ASSIGN*, LCONF_PARAM*))dlsym(gDllMcb[i].dllHandle, "DLLProcesarRequest");
    if (!gDllMcb[i].dllProcesarRequest){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"Error al obtener Address de DLLProcesarRequest");
      error = true;
    }

    /*[D]- Obtengo el Address de la DLLProcesarResponse */
    gDllMcb[i].dllProcesarResponse = 
          (short (*)(pstm_def*, tkn_area_def*, LCONF_ASSIGN*, LCONF_PARAM*))dlsym(gDllMcb[i].dllHandle, "DLLProcesarResponse");
    if (!gDllMcb[i].dllProcesarResponse){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"Error al obtener Address de DLLProcesarResponse");
      error = true;
    }

    /*[E]- Obtengo el Address de la DLLClose */
    gDllMcb[i].dllClose = 
          (short (*)(void))dlsym(gDllMcb[i].dllHandle, "DLLClose");
    if (!gDllMcb[i].dllClose){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"Error al obtener Address de DLLClose");
      error = true;
    }

    /*[F]- Obtengo el Address de la DLLInit */
    gDllMcb[i].dllInit = 
          (short (*)(SGDLL*, LCONF_ASSIGN*, LCONF_PARAM*))dlsym(gDllMcb[i].dllHandle, "DLLInit");
    if (!gDllMcb[i].dllInit){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"Error al obtener Address de DLLInit");
      error = true;
    }



    /* Ahora hago que cargue los Assigns y Params de cada libreria */
    rc = gDllMcb[i].dllInit(gSGDll, gLconfAssigns, gLconfParams);
    if (rc != 0){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"Error al cargar DLLInit");
      error = true;
    }

    if (error){
      memset(msg_err,' ', sizeof(msg_err));
      strcpy(msg_err, "Verificar el archivo 'MODULOS' para el registro: [");
      strncat(msg_err, gDllMcb[i].path_file, 24 );
      strncat(msg_err, "]", 1 );
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, msg_err);

      dlclose(gDllMcb[i].dllHandle);
      return 1;
    }



  }/* fin del bucle for */


  /*Cierro el archivo de MODULOS, ya tengo cargado en memoria los registros*/
  FILE_CLOSE_ (fd_modulos);

  return 0;

}



#pragma section create_STF
#pragma page "create_STF"
/*============================================================================*/
/* @Method: create_STF()                                                      */
/* @Brief:  Esta funcion se encarga de crear un nuevo archivo transaccional   */
/*          para dejar registradas las transacciones que sean necesarias.     */  
/* @Params: [none]                                                            */
/* @Return: [OK = 0] / [Error = 1]                                            */
/*============================================================================*/
short create_STF(char* fname, short fname_len){

short pri_extent     = 130000;
short sec_extent     = 130000;
short max_extent     = 25;
short rec_len        = 4048;
short data_block_len = 4096;
short file_type      = 2;  /*entry-sequenced*/
short error          = 0;

  error = FILE_CREATE_ (fname,
                        fname_len,
                        &fname_len,
                        ,
                        pri_extent, 
                        sec_extent,
                        max_extent, 
                        file_type,
                        , 
                        rec_len,
                        data_block_len);
  if (error == 0){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"STF creado: [%s]", fname );
  }else{
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,"[create_STF] (ERROR) al crear STF [%s]. Error Nro:[%d].", 
              fname, error);
    return error;
  }


  return FALSE;/*OK*/
}



#pragma section init_STF
#pragma page "init_STF"
/*============================================================================*/
/* @Method: init_STF()                                                        */
/* @Brief:  Inicia los nombre de los archivos de transacciones.               */
/* @Params: [none]                                                            */
/* @Return: [OK = 0] / [Error = 1]                                            */
/*============================================================================*/
short init_STF(){

const char assign[8] = "SAT-PTLF";

long long unDia = 86400000000; /* un dia expresado en microsegundos */
long long lCurrTmStmp;         /* actual timestamp */
long long tmstp_ant;           /* anterior timestamp */
long long tmstp_sig;           /* siguiente timestamp */

short shTimeStmp_ant[20];      /* Interpretacion del anterior timestamp */
short shTimeStmp_sig[20];      /* Interpretacion del siguiente timestamp */
short shTimeStamp[20];         /* Interpretacion del actual timestamp */

short encontrado = FALSE;
int inTmStmpRes;
short i;
short inicio;
short largo;
short error;
short number = -1;              

char szTmStmp[8];               /* timestamp actual en formato char */
char szTmStmp_ant[8];           /* timestamp anterior en formato char */
char szTmStmp_sig[8];           /* timestamp siguiente en formato char */

char file_tplt_lconf[35];       /* Template obtenido del LCONF */
char fname_actual[26 + 1];      /* Nombre de archivo Actual a crear */
char fname_anterior[26 + 1];    /* Nombre de archivo Anterior a crear */
char fname_siguiente[26 + 1];   /* Nombre de archivo Siguiente a crear */



  /*------------------------------------------------*/
  /*(0) Reservo memoria para guardar la informacion */
  /*------------------------------------------------*/
  gSTF = NULL;  /* Inicializo el puntero a nulo  */

  gSTF = (STF_GFD*)malloc(sizeof(STF_GFD));
  if (gSTF == NULL){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Fallo la asignacion de memoria inicial para gSTF");
    return 1;
  }

  /* Inicializo con ceros la memoria asignada */
  memset(gSTF, 0x00, sizeof(STF_GFD));


  /*-------------------------------------------------------------------------*/
  /*(1) busco el assign para obtener el path junto con el archivo de modulos */
  /*-------------------------------------------------------------------------*/
  i = 0;
  while (!encontrado){

    if (!memcmp(assign, gLconfAssigns[i].assign, sizeof(assign))){
    
      memcpy(file_tplt_lconf, gLconfAssigns[i].file_name, sizeof(file_tplt_lconf));
      encontrado = TRUE;
    
    }else{
      /*Incremento el indice*/
      i = i + 1;
    }
  }

  /*-------------------------------------------------------------------------*/
  /*(2) Obtengo la longitud exacta del path del archivo                      */
  /*-------------------------------------------------------------------------*/
  for (i=0; file_tplt_lconf[i] != NULL ; i++){
    if(file_tplt_lconf[i] == '$'){ 
      inicio = i; 
    }
  }
  
  largo = (i - inicio);

  /*-------------------------------------------------------------------------*/
  /*(3) Inicializo los Arrays de archivos con el nombre del LCONF            */
  /*-------------------------------------------------------------------------*/
  memset(fname_actual   ,0x00,sizeof(fname_actual));
  memset(fname_anterior ,0x00,sizeof(fname_anterior));
  memset(fname_siguiente,0x00,sizeof(fname_siguiente));

  strncpy(fname_actual   , &file_tplt_lconf[inicio], largo);
  strncpy(fname_anterior , &file_tplt_lconf[inicio], largo);
  strncpy(fname_siguiente, &file_tplt_lconf[inicio], largo);

  /*-------------------------------------------------------------------------*/
  /*(4) Obtengo el Time Stamp para formatear el nombre de los archivos.      */
  /*-------------------------------------------------------------------------*/
  memset(szTmStmp    ,0x00,sizeof(szTmStmp));
  memset(szTmStmp_ant,0x00,sizeof(szTmStmp_ant));
  memset(szTmStmp_sig,0x00,sizeof(szTmStmp_sig));

  memset(shTimeStamp   ,0x00,sizeof(shTimeStamp));
  memset(shTimeStmp_ant,0x00,sizeof(shTimeStmp_ant));
  memset(shTimeStmp_sig,0x00,sizeof(shTimeStmp_sig));

  lCurrTmStmp = JULIANTIMESTAMP(0);

  tmstp_ant = lCurrTmStmp - unDia; /* le resto un dia en microsegundos */ 
  tmstp_sig = lCurrTmStmp + unDia; /* le sumo  un dia en microsegundos */

  inTmStmpRes = INTERPRETTIMESTAMP(lCurrTmStmp,shTimeStamp);
  inTmStmpRes = INTERPRETTIMESTAMP(tmstp_ant,shTimeStmp_ant);
  inTmStmpRes = INTERPRETTIMESTAMP(tmstp_sig,shTimeStmp_sig);

  sprintf(szTmStmp    ,"%04d%02d%02d", shTimeStamp[0]/*año*/,shTimeStamp[1]/*mes*/,shTimeStamp[2]/*dia*/);
  sprintf(szTmStmp_ant,"%04d%02d%02d", shTimeStmp_ant[0]/*año*/,shTimeStmp_ant[1]/*mes*/,shTimeStmp_ant[2]/*dia*/);
  sprintf(szTmStmp_sig,"%04d%02d%02d", shTimeStmp_sig[0]/*año*/,shTimeStmp_sig[1]/*mes*/,shTimeStmp_sig[2]/*dia*/);

  /*-------------------------------------------------------------------------*/
  /*(5)$DATA26.DVL1DATA.SLYYMMDD armo el nombre con la fecha con el template */
  /*-------------------------------------------------------------------------*/
  /*-------------------------------------*/
  /* Armo el file name del dia anterior  */
  strncpy(&fname_anterior[19], &szTmStmp_ant[2] , 2);
  strncpy(&fname_anterior[21], &szTmStmp_ant[4] , 2);
  strncpy(&fname_anterior[23], &szTmStmp_ant[6] , 2);

  /* Guardo la fecha en formato yymmdd */
  strncpy(gSTF->fecha_ant, &fname_anterior[19] , 6);

  error = validar_STF(fname_anterior, largo);
  if (error){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Error en STF Anterior [%s]. Error Nro:[%d].", 
                fname_anterior, error);
    return error;
  }

  /*-------------------------------------*/
  /* Armo el file name del dia Actual    */
  strncpy(&fname_actual[19], &szTmStmp[2] , 2);
  strncpy(&fname_actual[21], &szTmStmp[4] , 2);
  strncpy(&fname_actual[23], &szTmStmp[6] , 2);

  /* Guardo la fecha en formato yymmdd */
  strncpy(gSTF->fecha_act, &fname_actual[19] , 6);

  error = validar_STF(fname_actual, largo);
  if (error){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Error en STF Actual [%s]. Error Nro:[%d]. ", 
                fname_actual, error);
    return error;
  }

  /*-------------------------------------*/
  /* Armo el file name del dia Siguiente */
  strncpy(&fname_siguiente[19], &szTmStmp_sig[2] , 2);
  strncpy(&fname_siguiente[21], &szTmStmp_sig[4] , 2);
  strncpy(&fname_siguiente[23], &szTmStmp_sig[6] , 2);  

  /* Guardo la fecha en formato yymmdd */
  strncpy(gSTF->fecha_sig, &fname_siguiente[19] , 6);

  error = validar_STF(fname_siguiente, largo);
  if (error){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Error en STF Siguiente [%s]. Error Nro:[%d]. ", 
                fname_siguiente, error);
    return error;
  }

  /*-------------------------------------------------------------------------*/
  /*(6) Actualizo la estructura STF Glogal File Data                         */
  /*-------------------------------------------------------------------------*/
  gSTF->largo = largo;
  strncpy(gSTF->fname_ant, fname_anterior , largo);
  strncpy(gSTF->fname_act, fname_actual   , largo);
  strncpy(gSTF->fname_sig, fname_siguiente, largo);


  /*-------------------------------------------------------------------------*/
  /*(7) Abro los archivos y gurado los files descriptor en la estructura     */
  /*-------------------------------------------------------------------------*/
  number = 0;
  error = open_STF(fname_anterior, largo, number);
  if (error){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Error al Abrir el archivo STF [%s]. Error Nro:[%d].", 
                fname_anterior, error);
    return error;
  }

  number = 1;
  error = open_STF(fname_actual, largo, number);
  if (error){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Error al Abrir el archivo STF [%s]. Error Nro:[%d]."
                , fname_actual, error);
    return error;
  }

  number = 2;
  error = open_STF(fname_siguiente, largo, number);
  if (error){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "Error al Abrir el archivo STF [%s]. Error Nro:[%d].", 
                fname_siguiente, error);
    return error;
  }

  /*-------------------------*/
  /* Inicializacion completa */
  /*-------------------------*/
  LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, "STF's INITIALIZED");

  return FALSE;/*OK*/
}



#pragma section validar_STF
#pragma page "validar_STF"
/*============================================================================*/
/* @Method: validar_STF()                                                     */
/* @Brief:  valido la existencia de los archivos antes de crearlos            */
/* @Params: [none]                                                            */
/* @Return: [OK = 0] / [Error = 1]                                            */
/*============================================================================*/
short validar_STF(char* fname, short len){

short error = 0;

  /*-------------------------------------------------------------------------*/
  /*(5)Realizo la validacion de la existencia de los archivos                */
  /*-------------------------------------------------------------------------*/
  error = FILE_GETINFOBYNAME_ ( fname, len );
  if (error == 0){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "El archivo STF [%s] se encuentra creado", 
                fname);
  }else{
    /* Si no existe debo crearlo */
    error = create_STF(fname, len);
    if (error){ return error; }
  }


  return FALSE;/*OK*/
}




#pragma section open_STF
#pragma page "open_STF"
/*============================================================================*/
/* @Method: open_STF()                                                        */
/* @Brief:  Realiza la apertura del archivo de transacciones y guarda el      */
/*          'file_num' en una variable global .                               */
/* @Params: [none]                                                            */
/* @Return: [OK = 0] / [Error = 1]                                            */
/*============================================================================*/
short open_STF(char* fname, short len, short number){

short error = 0;
short fd;

  error = FILE_OPEN_(fname, len, &fd);
  if (error != 0){
    LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                    "No se pudo Abrir el archivo de Modulos Dinamicos.");
    return TRUE;
  }


  /* Actualizo los Flie Descriptor en la estructura global */
  switch (number){
    case 0:
      gSTF->fd_ant = fd;
      break;
    case 1:
      gSTF->fd_act = fd;
      break;
    case 2:
      gSTF->fd_sig = fd;
      break;
    default:
      return TRUE; /* ERROR !!! */
      break;
  }


  return FALSE;/*OK*/
}



#pragma section close_STF
#pragma page "close_STF"
/*============================================================================*/
/* @Method: close_STF()                                                       */
/* @Brief:  Realiza el cierre del archivo de transacciones.                   */
/* @Params: [none]                                                            */
/* @Return: [OK = 0] / [Error = 1]                                            */
/*============================================================================*/
short close_STF(short number){

short error = 0;


  switch (number){
    case 0:
        error = FILE_CLOSE_ (gSTF->fd_ant); /* Cierro el archivo Anterior  */
        if (error != 0){
          LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                      "No se pudo Cerrar el archivo STF de Ayer.");
          return TRUE;
        }

        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                      "Se Cerro el archivo STF [%s].", gSTF->fname_ant );

      break;
    case 1:
        FILE_CLOSE_ (gSTF->fd_act); /* Cierro el archivo Actual    */
        if (error != 0){
          LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                      "No se pudo Cerrar el archivo STF Actual.");
          return TRUE;
        }

        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                      "Se Cerro el archivo STF [%s].", gSTF->fname_act );
      break;
    case 2:
        FILE_CLOSE_ (gSTF->fd_sig); /* Cierro el archivo Siguiente */
        if (error != 0){
          LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                      "No se pudo Cerrar el archivo STF de Mañana.");
          return TRUE;
        }

        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                      "Se Cerro el archivo STF [%s].", gSTF->fname_sig );

      break;
    default:
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                  "Verificar el archivo STF que desea cerrar." );
      return TRUE; /* ERROR !!! */
      break;
  }




  return FALSE;/*OK*/
}


