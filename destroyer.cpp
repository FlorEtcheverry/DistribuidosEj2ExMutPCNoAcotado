/* 
 * File:   destroyer.cpp
 * Author: knoppix
 *
 * Created on March 29, 2015, 10:53 PM
 */

#include <cstdlib>
#include <sys/shm.h>
#include "Semaforo.h"
#include "Logger.h"

using namespace std;

/*
 * destruye los ipc
 */
int main(int argc, char** argv) {

    Logger::init(PATH_LOG,argv[0]);
    
    //destruir mutex
    Semaforo mutex = Semaforo(PATH_IPC_MUTEX.c_str());
    mutex.destroy();
    (Logger::getLogger())->escribir(MSJ,"Mutex destruido.");
    
    //destruir sem museo lleno
    Semaforo mutex = Semaforo(PATH_IPC_MUSEOLLENO.c_str());
    mutex.destroy();
    (Logger::getLogger())->escribir(MSJ,"Semaforo de museo lleno destruido.");
    
    //obtener memoria compartida
    key_t key = ftok(PATH_IPC_SHM.c_str(),SHM);
    int shm_id = shmget(key,sizeof(MUSEO),0666);
    if (shm_id == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la memoria compartida para destruirla.");
        Logger::destroy();
        exit(1);
    }
    
    MUSEO* museo_shm = static_cast<MUSEO*>(shmat(shm_id,0,0));
    if (museo_shm == (MUSEO*)(-1)) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo inicializar la memoria compartida.");
        Logger::destroy();
        exit(1);
    }
    
    //obtengo la cant de puertas
    int cantidad_puertas = museo_shm->cant_puertas;
    
    int res = shmdt(museo_shm);
    if (res < 0) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo desatachear la memoria compartida al inicializar.");
        Logger::destroy();
        exit(1);
    }
    
    //elimino la memoria
    int res = shmctl(shm_id,IPC_RMID,NULL);
    if (res == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo destruir la memoria compartida.");
        Logger::destroy();
        exit(1);
    }
    (Logger::getLogger())->escribir(MSJ,"Memoria compartida destruida.");
    
    //eliminar colas de puertas
    static char puerta[MAX_DIG_PUERTA];
    
    for (int i=0;i<cantidad_puertas;i++) {
        sprintf(puerta,"%d",i);
        
        //eliminar cola entrada
        key_t key = ftok(PATH_IPC_COLAENTRADA.c_str(),i);
        int id_cola = msgget(key,0);
        if (id_cola == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de entrada para la puerta "+puerta+" para eliminarla.");
            Logger::destroy();
            exit(1);
        }
        int res = msgctl(id_cola,IPC_RMID,NULL);
        if (res == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo destruir la cola de entrada para puerta "+puerta+".");
            Logger::destroy();
            exit(1);
        }
        Logger::getLogger()->escribir(MSJ,"Eliminada exitosamente cola de entrada para puerta "+puerta+".");

        //eliminar cola entrada respuesta
        key = ftok(PATH_IPC_COLAENTRADA_RESP.c_str(),i);
        id_cola = msgget(key,0);
        if (id_cola == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de entrada de respuesta para la puerta "+puerta+" para eliminarla.");
            Logger::destroy();
            exit(1);
        }
        res = msgctl(id_cola,IPC_RMID,NULL);
        if (res == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo destruir la cola de entrada de respuesta para puerta "+puerta+".");
            Logger::destroy();
            exit(1);
        }
        Logger::getLogger()->escribir(MSJ,"Eliminada exitosamente cola de entrada de respuesta para puerta "+puerta+".");

        //eliminar cola de salida
        key = ftok(PATH_IPC_COLASALIDA.c_str(),i);
        id_cola = msgget(key,0);
        if (id_cola == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de salida para la puerta "+puerta+" para eliminarla.");
            Logger::destroy();
            exit(1);
        }
        res = msgctl(id_cola,IPC_RMID,NULL);
        if (res == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo destruir la cola de salida para puerta "+puerta+".");
            Logger::destroy();
            exit(1);
        }
        Logger::getLogger()->escribir(MSJ,"Eliminada exitosamente cola de salida para puerta "+puerta+".");

        //eliminar cola salida respuesta
        key = ftok(PATH_IPC_COLASALIDA_RESP.c_str(),i);
        id_cola = msgget(key,0);
        if (id_cola == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de salida de respuesta para la puerta "+puerta+" para eliminarla.");
            Logger::destroy();
            exit(1);
        }
        res = msgctl(id_cola,IPC_RMID,NULL);
        if (res == -1) {
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo destruir la cola de salida de respuesta para puerta "+puerta+".");
            Logger::destroy();
            exit(1);
        }
        Logger::getLogger()->escribir(MSJ,"Eliminada exitosamente cola de salida de respuesta para puerta "+puerta+".");

    }
    
    
    
    
    (Logger::getLogger())->escribir(MSJ,"------------------------------------------------------");
    Logger::destroy();
    return 0;
}

