
#ifndef _SAT_H_
#define _SAT_H_

#include "lconf.h"

short loglevel = 0;

char szSysName[17];  /*esta me parece que vuela*/
char lconf_name[27]      ;
char collector_name[27]  ;

short procesar_mensaje(char* szSysName);

#endif