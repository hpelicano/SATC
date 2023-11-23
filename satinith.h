
#ifndef _SATINIT_H_
#define _SATINIT_H_

#include <lconf.h>
#include <flags.h>

//#define TRUE 1
//#define FALSE 0

/* Modo de Acceso a archivos */
#define Read_write 0
#define Read_only  1 
#define Write_only 2 

#ifndef _BADDLC_
#define _BADDLC_
    #include <baddlc>
#endif

#ifndef _TOKEN_H_
#define _TOKEN_H_
    #include <token.h>
#endif

extern short loglevel;

extern char LCONF_static_message_string[4096];

/* LConf */
LCONF_ASSIGN gLconfAssigns[64];
LCONF_PARAM gLconfParams[32];



/* SAT Global DLLs - Archivo MODULOS y LCONF*/
typedef struct _SGDLL{
    char  szSysName[17];    /* Nombre del Proceso */
    char  lconf_name[27];   /* Archivo de LCONF   */
    char  path_name[26];    /* Archivo de MODULOS */
    short path_name_len;
}SGDLL;

SGDLL* gSGDll;

/* DLL's Memory Control Block */
typedef struct _dll_mcb{
    char   prsp[4];
    char   status[1];
    char   path_file[26];
    void*  dllHandle;
    short (*dllInfo)(char* p_prsp);
    short (*dllProcesarRequest)(pstm_def* pstm, tkn_area_def* tokens, 
                                LCONF_ASSIGN* lconfAssigns, 
                                LCONF_PARAM* lconfParams);
    short (*dllProcesarResponse)(pstm_def* pstm, tkn_area_def* tokens, 
                                LCONF_ASSIGN* lconfAssigns, 
                                LCONF_PARAM* lconfParams);
    short (*dllClose)(void);
    short (*dllInit)(SGDLL* p_gSGDll, LCONF_ASSIGN* lconfAssigns, LCONF_PARAM* lconfParams);
}DLL_MCB;


DLL_MCB* gDllMcb;
short count_dlls_mcb;


/*---------------------------------------------------------*/
/* Satelite Transaction File STF - Global File Data - GFD  */
/*---------------------------------------------------------*/
typedef struct _stf{
    short largo;
    short fd_ant;
    short fd_act;
    short fd_sig;
    char fecha_ant[6];    /* Fecha de anterior 'YYMMDD' */
    char fecha_act[6];    /* Fecha de actual 'YYMMDD' */
    char fecha_sig[6];    /* Fecha de siguiente 'YYMMDD' */
    char fname_ant[26];
    char fname_act[26];
    char fname_sig[26];
}STF_GFD;

STF_GFD* gSTF; 


/* ----------------------------------------------------------------- */
/*  Prototipo de funciones                                           */
/* ----------------------------------------------------------------- */
short inicializar_Lconf(const char* szLncfName);
short load_dinamic_dlls();

/* Funciones para el manejo del archivo de Log de Transacciones */
short init_STF();
short validar_STF(char* fname, short len);
short create_STF(char* fname, short len);
short open_STF(char* fname, short len, short number);
short close_STF(short number);


#endif