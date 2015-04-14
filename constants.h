/* 
 * File:   constants.h
 * Author: knoppix
 *
 * Created on April 12, 2015, 12:59 PM
 */

#ifndef CONSTANTS_H
#define	CONSTANTS_H

typedef struct {
    int cant_personas;
    int abierto;
    int max_personas;
    int cant_puertas;
} MUSEO;

typedef struct {
    long mtype;
    long senderPid;
    int mensaje; 
} MENSAJE;

static const int MUSEO_INICIAL = 0; //empieza en 0 -no entra nadie hasta que abran el museo
static const int SEM_HAY_LUGAR = 1;

static const std::string PATH_LOG = "./log.log";
static const std::string SHM_INIT_FILE = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/init.conf";  //abierto(1 o 0) ; cant_personas en museo ; max_personas ; cant_visitantes ; cant_puertas (por linea)

static const std::string PATH_IPC_SHM = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1";
static const std::string PATH_IPC_MUTEX = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/mutex";
static const std::string PATH_IPC_MUSEOLLENO = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/museolleno";
static const std::string PATH_IPC_COLAENTRADA = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colaentrada";
static const std::string PATH_IPC_COLAENTRADA_RESP = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colaentradarespuesta";
static const std::string PATH_IPC_COLASALIDA = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colasalida";
static const std::string PATH_IPC_COLASALIDA_RESP = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colasalidarespuesta";
static const std::string PATH_IPC_COLAMUSEOCERRADO = "/home/knoppix/AlvarezEtcheverry/Ejercicio2/v1/colamuseocerrado";

static const int ERROR = 1;
static const int MSJ = 2;

static const int SEM = 10;
static const int SHM = 20;
static const int COLA_MUSEO_CERR = 30;

static const int MAX_DIG_PUERTA = 11;

static const char* PUERTA_ENTRADA_EXE = "./puertaEntrada";
static const char* PUERTA_SALIDA_EXE = "./puertaSalida";
static const char* CLON_PERSONA_EXE = "./clon";

static const int QUIERO_ENTRAR = 300;
static const int QUIERO_SALIR = 301;
static const int PODES_ENTRAR = 302;
static const int MUSEO_CERRADO = 303;
static const int PODES_SALIR = 304;


#endif	/* CONSTANTS_H */

