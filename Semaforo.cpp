/* 
 * File:   Semaforo.cpp
 * Author: knoppix
 * 
 * Created on March 28, 2015, 7:18 PM
 */

#include "Semaforo.h"

Semaforo::Semaforo(const char* sem_dir) {
    key_t key = ftok(sem_dir,SEM);
    this-> id = semget(key,1,0666|IPC_CREAT);
    if (id == -1){
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+"No se pudo abrir el semaforo.");
        exit(1);
    }
}

void Semaforo::init(int initial_value){
    union semnum {
		int val;
		struct semid_ds*buf;
		ushort*array;
	};
	semnum init ;
	init.val = initial_value;
	int resultado = semctl (this->id,0,SETVAL,init);
	if (resultado == -1){
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo inicializar el semaforo");
            exit(1);
	}
}
void Semaforo::p(){  //TODO errores de p y v? devolver? exit?
    struct sembuf operacion ;
    operacion.sem_num = 0; // nro sem
    operacion.sem_op = -1;
    operacion.sem_flg = 0;
    int resultado = semop(this-> id,&operacion,1) ;
    if (resultado == -1){
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" Error al esperar por el semaforo");
        exit(1);
    }
}
void Semaforo::v(){
    struct sembuf operacion ;
    operacion.sem_num = 0;
    operacion.sem_op = 1;
    operacion.sem_flg = 0;
    int resultado = semop(this-> id,&operacion,1) ;
    if (resultado == -1){
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" Error al levantar el semaforo");
        exit(1);
    }
}
void Semaforo::destroy(){
    int resultado = semctl(this->id,0,IPC_RMID);
    if (resultado == -1){
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" Error al destruir el semáforo.");
    }
}

Semaforo::~Semaforo() {
}