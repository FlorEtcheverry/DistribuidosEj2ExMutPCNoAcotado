/* 
 * File:   Semaforo.h
 * Author: knoppix
 *
 * Created on March 28, 2015, 7:18 PM
 */

#ifndef SEMAFORO_H
#define	SEMAFORO_H

#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include "Logger.h"

class Semaforo {
public:
    Semaforo(const char* sem_dir);
    virtual ~Semaforo();
    
    void init(int initial_value); //solo el que lo crea deberia llamar a esto
    void p(); //decrementa
    void v(); //incrementa
    void destroy(); //solo el que lo destruye deberia llamar a esto
    
private:
    int id;
};

#endif	/* SEMAFORO_H */

