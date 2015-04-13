/* 
 * File:   clon.cpp
 * Author: knoppix
 *
 * Created on April 12, 2015, 7:14 PM
 */

#include <cstdlib>
#include <signal.h>
#include "Logger.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    Logger::init(PATH_LOG,argv[0]);
    
    (Logger::getLogger())->escribir(MSJ,string("Clon de la persona ")+getppid()+" creado.");
    
    //obtener cola museo cerrado
    int key = ftok(PATH_IPC_COLAMUSEOCERRADO.c_str(),COLA_MUSEO_CERR);
    int cola_museo_cerrado = msgget(key,0666);
    if (cola_museo_cerrado == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo obtener la cola de aviso de museo cerrado en el clond de la persona "+getppid()+".");
        Logger::destroy();
        exit(1);
    }
    
    //leer 
    MENSAJE msj;
    ssize_t res = msgrcv(cola_museo_cerrado,&msj,sizeof(MENSAJE)-sizeof(long),0,0);
    if (res == -1) {//puede ser el fallo por el kill del otro proceso que salio del sleep
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo leer de la cola de museo cerrado en el clon de la persona "+getppid()+".");
        Logger::destroy();
        exit(1);
    }
    //mata al padre
    kill(getppid(),SIGUSR1);
    
    //pedir salir

    return 0;
}

