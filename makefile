# Makefile
#
# Ejecutar GMAKE en el subvolumen donde se encuentra el archivo MAKEFILE
# Opcionelmente se puede ejecutar
# GMAKE ALL -> es lo mismo que ejecutar GMAKE sin argumentos
# GMAKE CLEAN -> para borrar los objetos genenerados
# GMAKE REBUILD para recompilar todo
all     : SATMARK2
rebuild : clean all
clean   : $SYSTEM.SYS00.FUP PURGE DLLO SATPROCO SATINITO SATCO !
# -------------------------------------------------------
# Compilacion de objetos intermedios por cada Soruce Code
#--------------------------------------------------------
DLLO :   DLLC DLLH
         $SYSTEM.SYSTEM.CCOMP/IN DLLC/ DLLO;\
         SYMBOLS,OPTIMIZE 0,extensions,xmem,nolist,       \
         refaligned 8,fieldalign shared2, allow_cplusplus_comments, \
         SSV1 "$SYSTEM.SYSTEM", SSV10 "$DATA17.XNLIB",    \
		 SSV12 "$DATA26.DLLSIST", SSV20 "$SYSTEM.ZSYSDEFS", \
         SSV21 "$DATA26.CSMBSRC"
         
SATPROCO: SATPROC SATPROH
         $SYSTEM.SYSTEM.CCOMP/IN SATPROC/ SATPROCO;\
         SYMBOLS,OPTIMIZE 0,extensions,xmem,nolist,       \
         refaligned 8,fieldalign shared2, allow_cplusplus_comments, \
         SSV1 "$SYSTEM.SYSTEM", SSV10 "$DATA17.XNLIB",    \
		 SSV12 "$DATA26.DLLSIST", SSV20 "$SYSTEM.ZSYSDEFS", \
         SSV21 "$DATA26.CSMBSRC"
         
SATINITO: SATINITC SATINITH
         $SYSTEM.SYSTEM.CCOMP/IN SATINITC/ SATINITO;\
         SYMBOLS,OPTIMIZE 0,extensions,xmem,nolist,       \
         refaligned 8,fieldalign shared2, allow_cplusplus_comments, \
         SSV1 "$SYSTEM.SYSTEM", SSV10 "$DATA17.XNLIB",    \
		 SSV12 "$DATA26.DLLSIST", SSV20 "$SYSTEM.ZSYSDEFS", \
         SSV21 "$DATA26.CSMBSRC"

SATCO :  SATC SATH
         $SYSTEM.SYSTEM.CCOMP/IN SATC/ SATCO;\
         SYMBOLS,OPTIMIZE 0,extensions,xmem,nolist,       \
         refaligned 8,fieldalign shared2, allow_cplusplus_comments, \
         SSV1 "$SYSTEM.SYSTEM", SSV10 "$DATA17.XNLIB",    \
		 SSV12 "$DATA26.DLLSIST", SSV20 "$SYSTEM.ZSYSDEFS", \
         SSV21 "$DATA26.CSMBSRC"

# -----------------------------------------------
# Linker de objetos intermedios con las Librerias
#------------------------------------------------
SATMARK2  : DLLO SATPROCO SATINITO SATCO
        $SYSTEM.SYSTEM.XLD $SYSTEM.SYSTEM.CCPMAINX \
		-libvol $DATA17.XNLIB   \
		-lib $DATA17.XNLIB.XNLIBEN \
        -lib $DATA26.DLLSIST.DLLSIST \
        -lib XCREDLL            \
        -lib XCRTLDLL           \
        -lib XRLDDLL            \
        -verbose                \
        -set saveabend on       \
        -set inspect on         \
        -set highpin on         \
        -unres_symbols ignore   \
        -show_multiple_defs     \
         DLLO \
		 SATPROCO \
		 SATINITO \
		 SATCO \		
        -o SATMARK2
