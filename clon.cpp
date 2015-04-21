/* 
 * File:   clon.cpp
 * Author: knoppix
 *
 * Created on April 12, 2015, 7:14 PM
 */

#include <cstdlib>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include "Logger.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    Logger::init(PATH_LOG,argv[0]);
    static char* puerta_salir = argv[1];
    int nro_puerta_salir = atoi(argv[1]);
    
    static char ppid[MAX_DIG_PID];
    sprintf(ppid,"%d",getppid());
    static char pid[MAX_DIG_PID];
    sprintf(pid,"%d",getpid());
    (Logger::getLogger())->escribir(MSJ,string("Clon (pid: ")+pid+") de la persona "+ppid+" creado. Saldría por la puerta "+puerta_salir+".");
    
    //obtener cola museo cerrado
    int key = ftok(PATH_IPC_COLAMUSEOCERRADO.c_str(),COLA_MUSEO_CERR);
    int cola_museo_cerrado = msgget(key,0666);
    if (cola_museo_cerrado == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de aviso de museo cerrado en el clon (pid: "+pid+") de la persona "+ppid+".");
        Logger::destroy();
        exit(1);
    }
    
    //leer msj de que tiene que salir
    MENSAJE msj;
    ssize_t res = msgrcv(cola_museo_cerrado,&msj,sizeof(MENSAJE)-sizeof(long),0,0);
    if (res == -1) {//puede ser el fallo por el kill del otro proceso que salio del sleep
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo leer de la cola de museo cerrado en el clon (pid: "+pid+") de la persona "+ppid+".");
        Logger::destroy();
        exit(1);
    }
    //mata al padre
    kill(getppid(),SIGUSR1);
    
    /*obtener colas salida*/
    //obtiene cola escribir
    key = ftok(PATH_IPC_COLASALIDA.c_str(),nro_puerta_salir);
    int cola_salir_escribir = msgget(key,0666);
    if (cola_salir_escribir == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de salida para el clon (pid: "+pid+") de la persona "+ppid+".");
        Logger::destroy();
        exit(1);
    }
    //obtiene cola leer
    key = ftok(PATH_IPC_COLASALIDA_RESP.c_str(),nro_puerta_salir);
    int cola_salir_leer = msgget(key,0666);
    if (cola_salir_leer == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de salida a leer para el clon (pid: "+pid+") de la persona "+ppid+".");
        Logger::destroy();
        exit(1);
    }
    
    //pedir salir
    MENSAJE msj_salir;
    msj_salir.mensaje = QUIERO_SALIR;
    msj_salir.mtype = 1;
    msj_salir.senderPid = getpid();
    res = msgsnd(cola_salir_escribir,&msj_salir,sizeof(MENSAJE)-sizeof(long),0);
    if (res == -1){
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo escribir en la cola de salir en el clon (pid: "+pid+") de la persona "+ppid+".");
        Logger::destroy();
        exit(1);
    }
    (Logger::getLogger())->escribir(MSJ,string("Clon (pid: ")+pid+") de la persona "+ppid+" quiere salir por la puerta"+puerta_salir+".");

    //leo rta
    MENSAJE resp;
    ssize_t leido = msgrcv(cola_salir_leer,&resp,sizeof(MENSAJE)-sizeof(long),getpid(),0);
    if (leido == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo leer de la cola de salir en el clon (pid: "+pid+") de la persona "+pid+".");
        Logger::destroy();
        exit(1);
    }
    if (resp.mensaje == PODES_SALIR) {
        (Logger::getLogger())->escribir(MSJ,string("Clon (pid: ")+pid+") de la persona "+ppid+" ya salió del museo.");
        Logger::destroy();
        return 0;
    } else {
        (Logger::getLogger())->escribir(ERROR,string("Clon (pid: ")+pid+") de la persona "+ppid+" no recibió autorización pasa salir.");
    }

    Logger::destroy();
    return 0;
}