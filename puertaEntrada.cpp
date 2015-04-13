/* 
 * File:   main.cpp
 * Author: knoppix
 *
 * Created on April 12, 2015, 12:19 PM
 */

#include <cstdlib>
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
    
    (Logger::getLogger())->escribir(MSJ,string("Puerta de entrada numero ")+puerta+" creada.");
    
    //obtiene mutex
    Semaforo mutex = Semaforo(PATH_IPC_MUTEX.c_str());
    
    //obtiene semaforo lugar en museo
    Semaforo sem_hay_lugar = Semaforo(PATH_IPC_MUSEOLLENO.c_str());
    
    //obtiene la mem compartida
    key_t key = ftok(PATH_IPC_SHM.c_str(),SHM);
    int shm_id = shmget(key,sizeof(MUSEO),0666);
    if (shm_id == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la memoria compartida en la puerta de entrada "+puerta+".");
        Logger::destroy();
        exit(1);
    }
    MUSEO* museo_shm = static_cast<MUSEO*>(shmat(shm_id,0,0));
    if (museo_shm == (MUSEO*)(-1)) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo attachear a la memoria compartida la puerta de entrada "+puerta+".");
        Logger::destroy();
        exit(1);
    }

    //obtiene cola recibir
    key = ftok(PATH_IPC_COLAENTRADA.c_str(),nro_puerta);
    int cola_recibir = msgget(key,0666);
    if (cola_recibir == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de entrada para la puerta "+puerta+".");
        shmdt(museo_shm);
        Logger::destroy();
        exit(1);
    }
    
    //obtiene cola responder
     key = ftok(PATH_IPC_COLAENTRADA_RESP.c_str(),nro_puerta);
    int cola_responder = msgget(key,0666);
    if (cola_responder == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de entrada de respuesta para la puerta "+puerta+".");
        shmdt(museo_shm);
        Logger::destroy();
        exit(1);
    }
    
    while (true) {
        
        //lee peticion en la cola
        MENSAJE msj;
        ssize_t res = msgrcv(cola_recibir,&msj,sizeof(MENSAJE)-sizeof(long),0,0);
        if (res == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo leer de la cola de entrada en la puerta "+puerta+".");
            Logger::destroy();
            exit(1);
        }
        if (msj.mensaje == QUIERO_ENTRAR) {
            
            (Logger::getLogger())->escribir(MSJ,string("Persona con pid ")+msj.senderPid+" quiere entrar al museo por la puerta "+puerta+".");
                
            sem_hay_lugar.p(); 
            mutex.p();
            
            if (museo_shm->abierto) {
                
                //si esta persona entra en el museo, pongo q hay lugar
                if (((museo_shm->cant_personas) + 1) < (museo_shm->max_personas)){
                    sem_hay_lugar.v();
                    (Logger::getLogger())->escribir(MSJ,string("Va a entrar una persona por la puerta ")+puerta+". Sigue habiendo lugar en el museo.");
                }
                (museo_shm->cant_personas)++; //agrego la persona al museo
                
                 //le mando el mensaje que puede pasar
                MENSAJE rta;
                rta.mtype = msj.senderPid;
                rta.mensaje = PODES_ENTRAR;
                rta.senderPid = getpid();
                int res = msgsnd(cola_responder,&rta,sizeof(MENSAJE)-sizeof(long),0);
                if (res == -1){
                    (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo escribir en la cola de entrada en la puerta "+puerta+".");
                    shmdt(museo_shm);
                    Logger::destroy();
                    exit(1);
                }
                (Logger::getLogger())->escribir(MSJ,string("Respondido a persona con pid ")+msj.senderPid+" que puede entrar al museo por la puerta "+puerta+".");
                
            } else { //museo cerrado
                //le mando el msj de que el museo esta cerrado
                MENSAJE rta;
                rta.mtype = msj.senderPid;
                rta.mensaje = MUSEO_CERRADO;
                rta.senderPid = getpid();
                int res = msgsnd(cola_responder,&rta,sizeof(MENSAJE)-sizeof(long),0);
                if (res == -1){
                    (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo escribir en la cola de entrada en la puerta "+puerta+".");
                    shmdt(museo_shm);
                    Logger::destroy();
                    exit(1);
                }
                (Logger::getLogger())->escribir(MSJ,string("Respondido a persona con pid ")+msj.senderPid+" que no puede entrar al museo por la puerta "+puerta+", porque está cerrado.");
                //pongo que hay lugar en el museo
                sem_hay_lugar.v();
            }
            mutex.v();
        }
    }
    
    //desatacheo de la memoria
    int res = shmdt(museo_shm);
    if (res < 0) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo desatachear la memoria compartida en la puerta de entrada "+puerta+".");
        Logger::destroy();
        exit(1);
    }
    Logger::destroy();
    return 0;
}

