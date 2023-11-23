#ifndef _SATPRO_H_
#define _SATPRO_H_

#include <token.h>
#include <msg.h>


pstm_def* pstm;
tkn_area_def* tkn_area;

short procesar_comandos(char* rcvMessage, short rcvMessageLen);
short procesar_mensaje(char* rcvMessage);
short procesar_mensaje_red(char* rcvMessage, short rcvMessageLen, char* srcName);
short Write_Msg_to_STF(short number, pstm_def* p_pstm, short largo);

#endif

