#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <cextdecs>
#include <tal.h>

#include <lconf.h>
#include "dll.h"
#include <logutil.h>
#include <zsysc>

#define extptr

#include<xncext>

extern char lconf_name[27];

#pragma section ProcesarRequest
#pragma page "ProcesarRequest"
/*============================================================================*/
/* @Method:  */
/* @Brief:   */
/* @Params:  */
/* @Return:  */
/*============================================================================*/
short ProcesarRequest(pstm_def* pstm, short count_dlls_mcb, tkn_area_def* tokens ){

csm_sat_tkn_def* tkn_q0; /* Token Q0 general del Satelite */

char tkn_id[2] = "Q0";
char prodSubprod[4] = "    ";
short i = 0;
short encontrado = false;
short rc = 0;
short tkn_len;
short stat;



    /*(1) Obtener el PrSp del Token 'Q0' */
    tkn_q0 = NULL;

    tkn_q0 = tkn_get( tokens, tkn_id, &stat, &tkn_len );

    if (tkn_len > 0 && stat == tkn_compl_ok_l){
        memcpy(prodSubprod, tkn_q0->info, sizeof(prodSubprod) );
    }else{
        LOGUTIL_Log(LLINFO, MSG_FREE_FORM, 
        "[ProcesarRequest]: No se pudo obtener [Prod/Subprod] del token Q0.");
        return ORIGEN_ERROR;    
    }

    /*(2) Routing DLL Dinamica */
    for(i=0; i<count_dlls_mcb; i++){
        if(!memcmp(gDllMcb[i].prsp, prodSubprod, sizeof(prodSubprod))){
            encontrado = true;
            break;
        }
    }

    /* Si encuentro el registro en la matriz de Dlls en memoria, vefifico
        que dicha info sea la correspondiente al Address obtenido y que 
        se encuentre Activa                                              */
    if(encontrado && !memcmp(gDllMcb[i].status,"A",1) ){
        if (gDllMcb[i].dllInfo(prodSubprod)){
            /* Mando a procesar el mensaje a la DLL correspondiente */
            rc = gDllMcb[i].dllProcesarRequest(pstm, tokens, gLconfAssigns, gLconfParams);
        }else{
            LOGUTIL_Log(LLINFO, MSG_FREE_FORM, 
                    "[ProcesarRequest]: Error en la asignacion de Address en gDllMcb.");
            return ORIGEN_ERROR;    
        }
    }else{
        LOGUTIL_Log(LLINFO, MSG_FREE_FORM, 
                    "[ProcesarRequest]: Producto/Subproducto [%.4s] no se encuentra Activo.", prodSubprod);

        memcpy(pstm->typ,"0210", 4);              /* 0210 de respuesta       */
        memcpy(pstm->tran.resp_cde, "071", 3);    /* Invalid RUT Auth        */
        pstm->tran.u_user_fld2.csm_sat_flg = 'E'; /* Indico un flag de ERROR */
        return ORIGEN_ERROR;    
    }


    return ORIGEN;/*OK*/
}



#pragma section ProcesarResponse
#pragma page "ProcesarResponse"
/*============================================================================*/
/* @Method:  */
/* @Brief:   */
/* @Params:  */
/* @Return:  */
/*============================================================================*/
short ProcesarResponse(pstm_def* pstm,short count_dlls_mcb, tkn_area_def* tokens){

    LOGUTIL_Log(LLINFO, MSG_FREE_FORM, "LLegue a ProcesarResponse", NULL);
    
    
    return ORIGEN;/*OK*/
}




#pragma section ProcesarComandos
#pragma page "ProcesarComandos"
/*============================================================================*/
/* @Method:  */
/* @Brief:   */
/* @Params:  */
/* @Return:  */
/*============================================================================*/
short ProcesarComandos(char* p_msg_txtptr, short rcvMessageLen){
short rc = false;
short i;

    /*---------------------------------------------------------------------*/
    /* para evitar grabar con error, fuerzo todo a letras Mayusculas       */
    /*---------------------------------------------------------------------*/
    for (i=0; i < rcvMessageLen; i++){
        if( islower(p_msg_txtptr[i]) != 0 ){
            p_msg_txtptr[i] = (char)toupper(p_msg_txtptr[i]);
        }
    }


    /*---------------------------------------------------------------------*/
    /*  Comando para CRUD de DLLs                                          */
    /*---------------------------------------------------------------------*/
    if(!memcmp(p_msg_txtptr, "9500", 4 )){
        rc = abm_dlls_dinamicas(p_msg_txtptr, rcvMessageLen);
        if (rc!=0){
            LOGUTIL_Log(LLINFO, MSG_FREE_FORM, 
                        "[ProcesarComandos]:Ejecucion de comando Finalizado.");
            return true;
        }

        /* Si la ejecucion fue exitosa, se realiza un Warmboot del proceso */
        rc = warmboot_dlls(); 
        if(rc != 0){
            LOGUTIL_Log(LLINFO, MSG_FREE_FORM, 
                    "[ProcesarComandos]:Error al ejecutar WARMBOOT.");
            return true;
        }
    }

    /*---------------------------------------------------------------------*/
    /*  Comando Cutover para Warmboot de STF Files                         */
    /*---------------------------------------------------------------------*/
    if(!memcmp(p_msg_txtptr, "9502", 4 )){
        rc = warmboot_STF_Files();
        if (rc!=0){
            LOGUTIL_Log(LLINFO, MSG_FREE_FORM, 
                        "[ProcesarComandos]:Error al ejecutar 9502.");
            return true;
        }
    }


    /*---------------------------------------------------------------------*/
    /*  Comando para Listar registros                                      */
    /*---------------------------------------------------------------------*/
    if(!memcmp(p_msg_txtptr, "9510", 4 )){
        rc = listar_dlls(p_msg_txtptr, rcvMessageLen);
        if (rc!=0){
            LOGUTIL_Log(LLINFO, MSG_FREE_FORM, 
                        "[ProcesarComandos]:Ejecucion de comando Finalizado");
            return true;
        }
    }

    return false;/*OK*/

}

#pragma section warmboot_STF_Files
#pragma page "warmboot_STF_Files"
/*============================================================================*/
/* @Method:  */
/* @Brief:   */
/* @Params:  */
/* @Return:  */
/*============================================================================*/
short warmboot_STF_Files(){

short inReturn = 0;

    close_STF(0); /* Archivo de tx de ayer   */
    close_STF(1); /* Archivo de tx de hoy    */
    close_STF(2); /* Archivo de tx de mañana */

    /* Libero la memoria global de los archivos STF  */
    free(gSTF); 

    /*-------------------------------------*/
    /* Crecion del Archivo STF             */
    /*-------------------------------------*/
    inReturn = init_STF();
    if(inReturn == 10){
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, 
                    "[warmboot_STF_Files] (ERROR): Falla al crear STF");
        return true;
    }

    return false; /*OK*/
}




#pragma section listar_dlls
#pragma page "listar_dlls"
/*============================================================================*/
/* @Method:  */
/* @Brief:   */
/* @Params:  */
/* @Return:  */
/*============================================================================*/
short listar_dlls(char* p_msg_txtptr, short rcvMessageLen){

short i;
short cont_dlls_activas = 0;
char  prsp_key[4];

    /* Control de longitudes*/
    if (rcvMessageLen != 19){
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[listar_dlls]: (ERROR): Verificar Largo del comando.");
        return 1;
    }

    /* Control de la Accion a ejecutar*/
    if (strncmp(&p_msg_txtptr[0], DLL_LIST, 8) ){
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[listar_dlls]: (Error) Verificar la Accion del comando.");
        return 1;
    }

    /* Control de la variable '-name' del comando */
    if ( !strncmp(&p_msg_txtptr[9], "-NAME", 5) ){

        for(i=0;i<4;i++){
            if(!strncmp(&p_msg_txtptr[15+i]," ", 1)){
                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "[listar_dlls]: (Error) '-name' Datos Vacios.");
                return 1;
            }    
        }
        /* Si esta todo bien, copio el contenido. */
        strncpy(prsp_key, &p_msg_txtptr[15], 4);
    }else{
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[listar_dlls]: (Error) falta la variable '-name'.");
        return 1;
    }


    /*------------------------------------------------------------------------*/
    /* Listo de memoria todas las DLLs que se encuentran Activas              */
    /*------------------------------------------------------------------------*/
    if ( !strncmp(prsp_key, "****", 4) ){

        if (count_dlls_mcb == 0){
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
            "[listar_dlls]: El archivo MODULOS no contiene Registros.");
            return 1;
        }

        /* la veriable 'count_dlls_mcb' es global y contiene el total de DLLs */    
        for (i=0; i<count_dlls_mcb; i++){
            if( !strncmp(gDllMcb[i].status,"A",1)){
                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "DLL Activa: [%s]", gDllMcb[i].path_file );
                cont_dlls_activas++;
            }
        }

        if (cont_dlls_activas == 0){
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
            "[listar_dlls]: No DLLs Activas en el archivo MODULOS.");
            return 1;
        }

    }else{
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[listar_dlls]: (Error) '-name' debe contener '****' .");
        return 1;
    }


    return false; /*OK*/
}

#pragma section warmboot_dlls
#pragma page "warmboot_dlls"
/*============================================================================*/
/* @Method:  */
/* @Brief:   */
/* @Params:  */
/* @Return:  */
/*============================================================================*/
short warmboot_dlls(){
short rc;
short i;
    /* Primero cierro las librerias dinamicas para soltar los OBJs de las DLLs */
    for (i = 0; i < count_dlls_mcb; i++){
        if (dlclose(gDllMcb[i].dllHandle) == 0 ){
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
            "[warmboot_dlls]: DLL Close/Unload: [%s], [%s].[%d]", 
            gDllMcb[i].path_file, dlerror(),dlresultcode());
        }
    }

    /* Libero la memoria del Memory Control Block de las DLLs */
    free(gDllMcb);

    /* Reinicio la estructura del LCONF */
    if(inicializar_Lconf(lconf_name)){
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM, 
            "[warmboot_dlls]: Error al Reiniciar el LCONF.");
        return -1;
    }

    /* Reinicio el proceso de carga en memoria de las librerias */
    rc = load_dinamic_dlls();
    if (rc != 0){
        return true; /* Fail */
    }

    LOGUTIL_Log(LLINFO, MSG_FREE_FORM, 
        "[warmboot_dlls]:WARMBOOT de librerias Dinamicas realizado.");


    /* Success - OK */
    return false;

}


#pragma section abm_dlls_dinamicas
#pragma page "abm_dlls_dinamicas"
/*============================================================================*/
/* @Method:  */
/* @Brief:   */
/* @Params:  */
/* @Return:  */
/*============================================================================*/
short abm_dlls_dinamicas(char* p_msg_txtptr, short rcvMessageLen){
short rc = 0;
short i;
short file_eof = false;
short fd_modulos;
short f_error;
short read_count  = 31;
short write_count = 31;
short indice = 0;
short prsp_path_file_len;
short file_len     = 7;
short subvol_len   = 8;
short prsp_vol_len = 0;
short type_info;
unsigned short count_read;
unsigned short count_written;

char  prsp_key[4];
char  prsp_vol[8];
char  prsp_path_file[26];
char  new_record[31]; 

char  buffer[31];   
_cc_status cc;
    /*-----------------------------------------------------------*/
    /*(1)-Bloque de controles previos a la ejecucion del comando */
    /*-----------------------------------------------------------*/

    /* Control de longitudes*/
    if (rcvMessageLen > 32 || rcvMessageLen < 19){
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[abm_dlls_dinamicas]: (ERROR): Verificar Largo del comando.");
        return 1;
    }

    /* Control de la Accion a ejecutar*/
    if (strncmp(&p_msg_txtptr[0], DLL_LOAD, 8) &&
        strncmp(&p_msg_txtptr[0], DLL_SUSP, 8) &&
        strncmp(&p_msg_txtptr[0], DLL_ACTV, 8) &&
        strncmp(&p_msg_txtptr[0], DLL_DELT, 8) ){
        
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[abm_dlls_dinamicas]: (Error) Verificar la Accion del comando.");
        return 1;
    }

    /* Control de la variable '-name' del comando */
    if ( !strncmp(&p_msg_txtptr[9], "-NAME", 5) ){

        for(i=0;i<4;i++){
            if(!strncmp(&p_msg_txtptr[15+i]," ", 1)){
                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "[abm_dlls_dinamicas]: (Error) '-name' Datos Vacios.");
                return 1;
            }    
        }
        /* Si esta todo bien, copio el contenido. */
        strncpy(prsp_key, &p_msg_txtptr[15], 4);
    }else{
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[abm_dlls_dinamicas]: (Error) falta la variable '-name'.");
        return 1;
    }

    /*-----------------------------------------------------------*/
    /*(2)-Valido la apertura del archivo de MODULOS de las DLLs  */
    /*-----------------------------------------------------------*/

    /* Abro el archivo de MODULOS. Esta guardado en [ gSGDll ] */
    rc = FILE_OPEN_(gSGDll->path_name, gSGDll->path_name_len, &fd_modulos);
    if (rc != 0){
      LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
      "[abm_dlls_dinamicas]: Error al abrir el archivo MODULOS.");
      return 1;
    }

    /*Indistintamente que venga un ALTA u otro comando, valido siempre 
      la existencia de un posble registro                              */
    cc = KEYPOSITION ( fd_modulos, prsp_key, , , 2 /*exact*/ );
    if(cc != 0){
        FILE_GETINFO_ (fd_modulos, &f_error);
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[abm_dlls_dinamicas]: Error [ %s ] en Keyposition de archivo MODULOS."
        , f_error);
        return 1;
    }

    /*Realizo la lectura del registro. Si el mismo existe, entonces me 
      guardo la info para procesarla. Si el Keyposition está en el fin
      de archivo, significa que ese registro no existe.                */
    memset(buffer,' ',sizeof(buffer));

    cc = READX ( fd_modulos, buffer, read_count, &count_read);
    if(cc < 0 ){
        FILE_GETINFO_ (fd_modulos, &f_error);
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[abm_dlls_dinamicas]: Error [ %s ] en la lectura del registro."
        , f_error);
        return 1;
    }else if(cc > 0){
        file_eof = true;
    }

    /*-----------------------------------------------------------*/
    /*(3)-Derivo la accion segun el comando solicitado           */
    /*-----------------------------------------------------------*/

    /* --------------------------------------------------------------------
       Solicitud de Alta de una nueva DLLs en el archivo de MODULOS de DLLs
       -------------------------------------------------------------------- */
    if ( !strncmp(&p_msg_txtptr[0], DLL_LOAD, 8) && file_eof){

        /*Verifico primeramente la existencia de la libreria estandarizada 
          recuperando la variable '-vol' del comando */
        if ( !strncmp(&p_msg_txtptr[20], "-VOL", 4) ){

            if(!strncmp(&p_msg_txtptr[25], "$", 1)){
                indice = 25;
                while( strncmp(&p_msg_txtptr[indice], "\0", 1) /*&& (indice <= 32)*/ ){
                    prsp_vol_len = prsp_vol_len + 1;
                    indice = indice + 1;
                } 
            }else{
                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "[abm_dlls_dinamicas]: (Error) el Volumen debe comenzar con '$'.");
                return 1;
            }

            strncpy(prsp_vol, &p_msg_txtptr[25], prsp_vol_len);
            
            /*Construyo un Path estandar con los datos obtenidos */
            strcat(prsp_path_file, prsp_vol);
            strcat(prsp_path_file, ".DLLS");
            strcat(prsp_path_file, prsp_key);
            strcat(prsp_path_file, ".DLL");
            strcat(prsp_path_file, prsp_key);

            /* sumo 1 byte por cada punto */
            prsp_path_file_len = prsp_vol_len + 1 + subvol_len + 1 + file_len ;    
            
            /*Hago un filegetinfo() para validar la existencia del mismo*/
            rc = FILE_GETINFOBYNAME_ ( prsp_path_file,
                                       prsp_path_file_len,
                                       &type_info);
            if(rc != 0){
                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "[abm_dlls_dinamicas]: (Error) No existe el Objeto DLL. [%s]",
                prsp_path_file);
                return 1;    
            }

        }else{
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
            "[abm_dlls_dinamicas]: (Error) falta la variable '-VOL'.");
            return 1;
        }

        /*Con los controles previos OK, estoy en condiciones de dar de Alta 
          un nuevo registro en el archivo de MODULOS. Por lo tanto, construyo
          el buffer con el contenido del registro con el siguiente formato:
          "PRSP,S,$DATA26.DLLSPRSP.DLLPRSP" . La longitud max es de 31 bytes.
          Inicialmente los registros se cargaran con la funcionalidad SUSP
          para que en un paso posterior se habilite la funcionalidad       */

        strcat(new_record, prsp_key);
        strcat(new_record, ",S,");
        strcat(new_record, prsp_vol);
        strcat(new_record, ".DLLS");
        strcat(new_record, prsp_key);
        strcat(new_record, ".DLL");
        strcat(new_record, prsp_key);


        cc = WRITEX ( fd_modulos, new_record, write_count, &count_written);
        if(cc != 0){
            FILE_GETINFO_ (fd_modulos, &f_error);
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
             "[abm_dlls_dinamicas]: Error [ %d ] en LOAD DLL", f_error);
            return 1;
        }

        /*Si todo salio bien dejo un mensaje en el loguer */
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[abm_dlls_dinamicas]: Alta de Registro: %s", new_record);

    }else 
    if ( !strncmp(&p_msg_txtptr[0], DLL_LOAD, 8) && !file_eof){
        /*Si todo salio bien dejo un mensaje en el loguer */
        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
        "[abm_dlls_dinamicas]: Resgistro ya existente: %s", new_record);
        return 1;
    };


    /* -------------------------------------------------------------------
      Se solicito una suspencion de la libreria por lo tanto midifico el
      registro dejando una 'S' en el campo status:                      
      ------------------------------------------------------------------ */
    if ( (!strncmp(&p_msg_txtptr[0], DLL_SUSP, 8) || 
          !strncmp(&p_msg_txtptr[0], DLL_ACTV, 8)  ) && !file_eof){

        if(!strncmp(&p_msg_txtptr[0], DLL_SUSP, 8)){
            if(!strncmp(&buffer[5], "S", 1)){
                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "[abm_dlls_dinamicas]: El Producto solicitado ya estaba Suspendido: %s", buffer);
                return 1;
            }else{
                buffer[5] = 'S';
            }
        }    

        if(!strncmp(&p_msg_txtptr[0], DLL_ACTV, 8)){
            if(!strncmp(&buffer[5], "A", 1)){
                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
                "[abm_dlls_dinamicas]: El Producto solicitado ya estaba Activo: %s", buffer);
                return 1;
            }else{
                buffer[5] = 'A';
            }
        }    
        


        /* Realizo la actualizacion del registro con el buffer modificado. */
        cc = WRITEUPDATEX ( fd_modulos, buffer, write_count, &count_written);
        if(cc != 0){
            FILE_GETINFO_ (fd_modulos, &f_error);
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
             "[abm_dlls_dinamicas]: Error [ %s ] en SUSP/ACTV", f_error);
            return 1;
        }



        if(!strncmp(&p_msg_txtptr[0], DLL_SUSP, 8)){
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
            "[abm_dlls_dinamicas]: Registro Suspendido: %s", buffer);
        }

        if(!strncmp(&p_msg_txtptr[0], DLL_ACTV, 8)){
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
            "[abm_dlls_dinamicas]: Registro Activado: %s", buffer);
        }

    };

 
    /* -------------------------------------------------------------------- 
       Delete de libreria en el archivo de MODULOS de DLLs 
      --------------------------------------------------------------------*/
    if ( !strncmp(&p_msg_txtptr[0], DLL_DELT, 8) && !file_eof){

        /*Para borrar el registro se escribe por una longitud de 0 bytes */
        cc = WRITEUPDATEX ( fd_modulos, buffer, 0 /*bytes*/);
        if(cc != 0){
            FILE_GETINFO_ (fd_modulos, &f_error);
            LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
             "[abm_dlls_dinamicas]: Error [ %s ] en DELETE", f_error);
            return 1;
        }

        LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
            "[abm_dlls_dinamicas]: Registro Eliminado para: [%s]", prsp_key );

        for (i = 0; i < count_dlls_mcb; i++){
            if (!strncmp(&p_msg_txtptr[15], gDllMcb[i].prsp, 4)){

                dlclose(gDllMcb[i].dllHandle);
                LOGUTIL_Log(LLPANIC, MSG_FREE_FORM,
               "[abm_dlls_dinamicas]: DLL Unload: [%s], [%s].[%d]", gDllMcb[i].path_file, dlerror(),dlresultcode());
            
            }
        }
        

    };



    /*()-Cierro el archivo de MODULOS */
    FILE_CLOSE_ (fd_modulos);

    return false;/*OK*/

}


