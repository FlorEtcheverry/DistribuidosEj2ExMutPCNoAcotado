/* 
 * File:   persona.cpp
 * Author: knoppix
 *
 * Created on April 12, 2015, 6:19 PM
 */

#include <cstdlib>
#include <signal.h>
#include "Logger.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) { //recibe nro puerta para entrar, nro puerta para salir, tiempo en museo

    Logger::init(PATH_LOG,argv[0]);
    
    static char* puerta_entrar = argv[1];
    int nro_puerta_entrar = atoi(argv[1]);
    static char* puerta_salir = argv[2];
    int nro_puerta_salir = atoi(argv[2]);
    static char* tiempo_paseando = argv[3];
    int nro_tiempo_paseando = atoi(argv[3]);
    (Logger::getLogger())->escribir(MSJ,string("Hola! Soy persona ")+getpid()+". Quiero entrar por la puerta numero "+puerta_entrar+" y salir por la "+puerta_salir+". Voy a pasear "+tiempo_paseando+" segs.");
      
    
    /*obtener colas entrada*/
    //obtiene cola escribir
    int key = ftok(PATH_IPC_COLAENTRADA.c_str(),nro_puerta_entrar);
    int cola_entrar_escribir = msgget(key,0666);
    if (cola_entrar_escribir == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de entrada para la persona "+getpid()+".");
        Logger::destroy();
        exit(1);
    }
    //obtiene cola leer
    key = ftok(PATH_IPC_COLAENTRADA_RESP.c_str(),nro_puerta_entrar);
    int cola_entrar_leer = msgget(key,0666);
    if (cola_entrar_leer == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de entrada a leer para la persona "+getpid()+".");
        Logger::destroy();
        exit(1);
    }
    
    /*obtener colas salida*/
    //obtiene cola escribir
    key = ftok(PATH_IPC_COLASALIDA.c_str(),nro_puerta_salir);
    int cola_salir_escribir = msgget(key,0666);
    if (cola_salir_escribir == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de salida para la persona "+getpid()+".");
        Logger::destroy();
        exit(1);
    }
    //obtiene cola leer
    key = ftok(PATH_IPC_COLASALIDA_RESP.c_str(),nro_puerta_salir);
    int cola_salir_leer = msgget(key,0666);
    if (cola_salir_leer == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de salida a leer para la persona "+getpid()+".");
        Logger::destroy();
        exit(1);
    }
    
    //obtener cola museo cerrado
    key = ftok(PATH_IPC_COLAMUSEOCERRADO.c_str(),COLA_MUSEO_CERR);
    int cola_museo_cerrado = msgget(key,0666);
    if (cola_museo_cerrado == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de aviso de museo cerrado para la persona "+getpid()+".");
        Logger::destroy();
        exit(1);
    }
    
    //pedir entrar
    MENSAJE peticion_entrar;
    peticion_entrar.mtype = 0;
    peticion_entrar.mensaje = QUIERO_ENTRAR;
    peticion_entrar.senderPid = getpid();
    int res = msgsnd(cola_entrar_escribir,&peticion_entrar,sizeof(MENSAJE)-sizeof(long),0);
    if (res == -1){
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo escribir en la cola de entrada en la persona "+getpid()+".");
        Logger::destroy();
        exit(1);
    }
    (Logger::getLogger())->escribir(MSJ,string("Persona ")+getpid()+" pidió que quiere entrar al museo por la puerta "+puerta_entrar+".");
    
    //leer respuesta
    MENSAJE rta;
    ssize_t res = msgrcv(cola_entrar_leer,&rta,sizeof(MENSAJE)-sizeof(long),getpid(),0);
    if (res == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo leer de la cola de entrada en la persona "+getpid()+".");
        Logger::destroy();
        exit(1);
    }
    
    if (rta.mensaje == PODES_ENTRAR) {
        
        int pid_hijo;
        pid_hijo = fork();
        if (pid_hijo < 0) {
            (Logger::getLogger())->escribir(ERROR,string(" No se pudo crear el clon de la persona ")+getpid()+".");
            Logger::destroy();
            exit(1);
        }
        if (pid_hijo == 0) { //(hijo) clon
            execlp(CLON_PERSONA_EXE,CLON_PERSONA_EXE,(char*) 0);
            (Logger::getLogger())->escribir(ERROR,string(" No se pudo ejecutar el clon de la persona ")+getpid()+".");
            Logger::destroy();
            exit(1);
        }
        
        // (padre) persona
        sleep (nro_tiempo_paseando);
        kill(pid_hijo,SIGUSR1);
        
        //pedir salir
        
    }
    Logger::destroy();
    return 0;
}

