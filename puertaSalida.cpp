/* 
 * File:   puertaSalida.cpp
 * Author: knoppix
 *
 * Created on April 12, 2015, 5:52 PM
 */

#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "Logger.h"
#include "Semaforo.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    Logger::init(PATH_LOG,argv[0]);
    
    static char* puerta = argv[1];
    int nro_puerta = atoi(argv[1]);
    
    (Logger::getLogger())->escribir(MSJ,string("Puerta de salida número ")+puerta+" creada.");
    
    //obtiene mutex
    Semaforo mutex = Semaforo(PATH_IPC_MUTEX.c_str());
    
    //obtiene semaforo lugar en museo
    Semaforo sem_hay_lugar = Semaforo(PATH_IPC_MUSEOLLENO.c_str());
    
    //obtiene la mem compartida
    key_t key = ftok(PATH_IPC_SHM.c_str(),SHM);
    int shm_id = shmget(key,sizeof(MUSEO),0666);
    if (shm_id == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la memoria compartida en la puerta de salida "+puerta+".");
        Logger::destroy();
        exit(1);
    }
    MUSEO* museo_shm = static_cast<MUSEO*>(shmat(shm_id,0,0));
    if (museo_shm == (MUSEO*)(-1)) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo attachear a la memoria compartida la puerta de salida "+puerta+".");
        Logger::destroy();
        exit(1);
    }

    //obtiene cola recibir
    key = ftok(PATH_IPC_COLASALIDA.c_str(),nro_puerta);
    int cola_recibir = msgget(key,0666);
    if (cola_recibir == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de salida para la puerta "+puerta+".");
        shmdt(museo_shm);
        Logger::destroy();
        exit(1);
    }
    
    //obtiene cola responder
     key = ftok(PATH_IPC_COLASALIDA_RESP.c_str(),nro_puerta);
    int cola_responder = msgget(key,0666);
    if (cola_responder == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de salida de respuesta para la puerta "+puerta+".");
        shmdt(museo_shm);
        Logger::destroy();
        exit(1);
    }
    
    while (true) {
        
        //lee peticion en la cola
        MENSAJE msj;
        ssize_t res = msgrcv(cola_recibir,&msj,sizeof(MENSAJE)-sizeof(long),0,0);
        if (res == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo leer de la cola de salida en la puerta "+puerta+".");
            Logger::destroy();
            exit(1);
        }
        if (msj.mensaje == QUIERO_SALIR) { //TODO: falta eso de matar a las puertas
            
            static char sender[MAX_DIG_PID];
            sprintf(sender,"%ld",msj.senderPid);
            (Logger::getLogger())->escribir(MSJ,string("Persona con pid ")+sender+" quiere salir del museo por la puerta "+puerta+".");
                
            mutex.p();
            
            if (museo_shm->cant_personas == museo_shm->max_personas) { //el museo está lleno
                
                (museo_shm->cant_personas)--; //saco a la persona
                
                //pongo q hay lugar en el museo
                sem_hay_lugar.v();
                (Logger::getLogger())->escribir(MSJ,string("Va a salir una persona por la puerta ")+puerta+". Ahora hay lugar en el museo.");
            
            } else {
                (museo_shm->cant_personas)--; //saco a la persona
            }
            //le mando el mensaje que puede salir
            MENSAJE rta;
            rta.mtype = msj.senderPid;
            rta.mensaje = PODES_SALIR;
            rta.senderPid = getpid();
            int res = msgsnd(cola_responder,&rta,sizeof(MENSAJE)-sizeof(long),0);
            if (res == -1){
                (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo escribir en la cola de salida en la puerta "+puerta+".");
                shmdt(museo_shm);
                Logger::destroy();
                exit(1);
            }
            (Logger::getLogger())->escribir(MSJ,string("Respondido a persona con pid ")+sender+" que puede salir del museo por la puerta "+puerta+".");
            mutex.v();
        }
    }
    
    //desatacheo de la memoria
    int res = shmdt(museo_shm);
    if (res < 0) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo desatachear la memoria compartida en la puerta de salida "+puerta+".");
        Logger::destroy();
        exit(1);
    }
    Logger::destroy();
    return 0;
}


