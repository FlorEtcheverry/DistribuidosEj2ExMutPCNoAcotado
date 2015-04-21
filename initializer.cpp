/* 
 * File:   main.cpp
 * Author: knoppix
 *
 * Created on March 27, 2015, 8:31 PM
 */

#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "Logger.h"
#include "Semaforo.h"
#include "constants.h"

using namespace std;

/* carga los datos iniciales de config desde el archivo*/
void cargarConfig(MUSEO* museo_shm,int* cant_visitantes,int* cant_puertas){
    std::ifstream conf_file(SHM_INIT_FILE.c_str());
    if (!conf_file.is_open()) {
        (Logger::getLogger())->escribir(ERROR," No se pudo abrir el archivo de conf para inicializar la memoria compartida.");
        shmdt(museo_shm);
        Logger::destroy();
        exit(1);
    }
    //abierto(1 o 0) ; cant_personas en museo ; max_personas ; cant_visitantes ; cant_puertas (por linea)
    std::string linea;
    
    getline(conf_file,linea); //TODO: chequeo de errores
    (museo_shm->abierto) = atoi(linea.c_str());
    if (museo_shm->abierto) {
        Logger::getLogger()->escribir(MSJ,string("Inicializado museo abierto "));
    } else {
        Logger::getLogger()->escribir(MSJ,string("Inicializado museo cerrado "));
    }
    
    getline(conf_file,linea);
    (museo_shm->cant_personas) = atoi(linea.c_str());
    Logger::getLogger()->escribir(MSJ,string("con ")+linea.c_str()+" personas dentro.");
    
    getline(conf_file,linea);
    (museo_shm->max_personas) = atoi(linea.c_str());
    Logger::getLogger()->escribir(MSJ,string("La maxima cantidad de personas permitidas en el museo es ")+linea.c_str()+".");
    
    getline(conf_file,linea);
    *cant_visitantes = atoi(linea.c_str());
    Logger::getLogger()->escribir(MSJ,string("Va a haber ")+linea.c_str()+" personas.");
    
    getline(conf_file,linea);
    *cant_puertas = atoi(linea.c_str());
     (museo_shm->cant_puertas) = atoi(linea.c_str());
    Logger::getLogger()->escribir(MSJ,string("Hay ")+linea.c_str()+" puertas.");
        
    conf_file.close(); //TODO: chequeo de errores
}

/*crear cola de entrada para la puerta indicada*/
int crearColaEntrada(int nro_puerta) {
    static char puerta[MAX_DIG_PUERTA];
    sprintf(puerta,"%d",nro_puerta);
    
    key_t key = ftok(PATH_IPC_COLAENTRADA.c_str(),nro_puerta);
    int id_cola = msgget(key,IPC_CREAT|IPC_EXCL|0666);
    if (id_cola == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo crear la cola de entrada para la puerta "+puerta+".");
        Logger::destroy();
        exit(1);
    }
    Logger::getLogger()->escribir(MSJ,string("Creada exitosamente cola de entrada para puerta ")+puerta+".");
    return id_cola;
}

int crearColaEntradaRta(int nro_puerta) {
    static char puerta[MAX_DIG_PUERTA];
    sprintf(puerta,"%d",nro_puerta);
    
    key_t key = ftok(PATH_IPC_COLAENTRADA_RESP.c_str(),nro_puerta);
    int id_cola = msgget(key,IPC_CREAT|IPC_EXCL|0666);
    if (id_cola == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo crear la cola de entrada de respuesta para la puerta "+puerta+".");
        Logger::destroy();
        exit(1);
    }
    Logger::getLogger()->escribir(MSJ,string("Creada exitosamente cola de entrada de respuesta para puerta ")+puerta+".");
    return id_cola;
}

/*crear cola de salida para la puerta indicada*/
int crearColaSalida(int nro_puerta) {
    static char puerta[MAX_DIG_PUERTA];
    sprintf(puerta,"%d",nro_puerta);
    
    key_t key = ftok(PATH_IPC_COLASALIDA.c_str(),nro_puerta);
    int id_cola = msgget(key,IPC_CREAT|IPC_EXCL|0666);
    if (id_cola == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo crear la cola de salida para la puerta "+puerta+".");
        Logger::destroy();
        exit(1);
    }
    Logger::getLogger()->escribir(MSJ,string("Creada exitosamente cola de salida para puerta ")+puerta+".");
    return id_cola;
}

int crearColaSalidaRta(int nro_puerta) {
    static char puerta[MAX_DIG_PUERTA];
    sprintf(puerta,"%d",nro_puerta);
    
    key_t key = ftok(PATH_IPC_COLASALIDA_RESP.c_str(),nro_puerta);
    int id_cola = msgget(key,IPC_CREAT|IPC_EXCL|0666);
    if (id_cola == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo crear la cola de salida de respuesta para la puerta "+puerta+".");
        Logger::destroy();
        exit(1);
    }
    Logger::getLogger()->escribir(MSJ,string("Creada exitosamente cola de salida de respuesta para puerta ")+puerta+".");
    return id_cola;
}

/*
 * crea los ipcs necesarios, carga datos iniciales desde arch de config y lanza los procesos
 */
int main(int argc, char** argv) {
  
    Logger::init(PATH_LOG,argv[0]);
    (Logger::getLogger())->escribir(MSJ,"--Iniciando programa--");
    
    /*crear la mem compartida*/
    key_t key = ftok(PATH_IPC_SHM.c_str(),SHM);
    int shm_id = shmget(key,sizeof(MUSEO),IPC_CREAT|IPC_EXCL|0666);
    if (shm_id == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo crear la memoria compartida.");
        Logger::destroy();
        exit(1);
    }
    Logger::getLogger()->escribir(MSJ,"Creada memoria compartida.");
    
    /*inicializar mem compartida (att,read,det) y cargar parametros de conf*/
    MUSEO* museo_shm = static_cast<MUSEO*>(shmat(shm_id,0,0));
    if (museo_shm == (MUSEO*)(-1)) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo inicializar la memoria compartida.");
        Logger::destroy();
        exit(1);
    }
    
    int cant_visitantes; //cant de personas que van a ser creadas
    int cant_puertas;
    cargarConfig(museo_shm,&cant_visitantes,&cant_puertas);
    
    int res = shmdt(museo_shm);
    if (res < 0) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo desatachear la memoria compartida al inicializar.");
        Logger::destroy();
        exit(1);
    }
    
    /*crear e inicializar mutex*/
    Semaforo mutex = Semaforo(PATH_IPC_MUTEX.c_str());
    mutex.init(MUSEO_INICIAL);
    Logger::getLogger()->escribir(MSJ,"Inicializado el mutex.");
    
    /*crear e inicializar semaforo museo lleno*/
    Semaforo sem_museo_lleno = Semaforo(PATH_IPC_MUSEOLLENO.c_str());
    sem_museo_lleno.init(SEM_HAY_LUGAR);
    Logger::getLogger()->escribir(MSJ,"Inicializado el semaforo de control de lugar en el museo.");
    
    /*crear colas y puertas*/
    int child_pid;
    static char nro_puerta[MAX_DIG_PUERTA];
    //static char nro_personas_por_puerta[MAX_DIG_PERS_POR_PUERTA];
    
    //sprintf(nro_personas_por_puerta,"%d",cant_visitantes); //convertir cant_vis de int a char*
    
    for (int i=0;i<cant_puertas;i++) {
        
        sprintf(nro_puerta,"%d",i);
        (Logger::getLogger())->escribir(MSJ,string("Creando puerta de entrada numero ")+nro_puerta+".");
        
        //CREAR colas y puerta de entrada
        crearColaEntrada(i);
        crearColaEntradaRta(i);
        
        child_pid = fork();
        if (child_pid < 0) {
            (Logger::getLogger())->escribir(ERROR,string(" No se pudo crear la puerta de entrada ")+nro_puerta+".");
            Logger::destroy();
            exit(1);
        }
        if (child_pid == 0) { //(hijo) puerta de entrada
            execlp(PUERTA_ENTRADA_EXE,PUERTA_ENTRADA_EXE,nro_puerta,(char*) 0);
            (Logger::getLogger())->escribir(ERROR,string(" No se pudo ejecutar la puerta de entrada ")+nro_puerta+".");
            Logger::destroy();
            exit(1);
        }
        
        //CREAR colas y puerta de salida
        (Logger::getLogger())->escribir(MSJ,string("Creando puerta de salida numero ")+nro_puerta+".");
        
        crearColaSalida(i);
        crearColaSalidaRta(i);
        
        child_pid = fork();
        if (child_pid < 0) {
            (Logger::getLogger())->escribir(ERROR,string(" No se pudo crear la puerta de salida ")+nro_puerta+".");
            Logger::destroy();
            exit(1);
        }
        if (child_pid == 0) { //(hijo) puerta de salida
            execlp(PUERTA_SALIDA_EXE,PUERTA_SALIDA_EXE,nro_puerta,(char*) 0);
            (Logger::getLogger())->escribir(ERROR,string(" No se pudo ejecutar la puerta de salida ")+nro_puerta+".");
            Logger::destroy();
            exit(1);
        }
    }
    (Logger::getLogger())->escribir(MSJ,string("---Terminada la creación de las puertas.---"));
    
    /*crear cola de museo cerrado*/
    key = ftok(PATH_IPC_COLAMUSEOCERRADO.c_str(),COLA_MUSEO_CERR);
    int cola_museo_cerrado = msgget(key,IPC_CREAT|IPC_EXCL|0666);
    if (cola_museo_cerrado == -1) {
        (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+" No se pudo crear la cola de aviso de museo cerrado.");
        Logger::destroy();
        exit(1);
    }
    Logger::getLogger()->escribir(MSJ,"Creada la cola de aviso de museo cerrado.");
    
    /*crear personas*/
    int pid_hijo_persona;
    static char nro_persona[MAX_DIG_PUERTA];
    srand(time(NULL));
    
    (Logger::getLogger())->escribir(MSJ,string("--Creando personas---"));
    for (int i=0;i<=cant_visitantes;i++){
        sprintf(nro_persona,"%d",i);
        (Logger::getLogger())->escribir(MSJ,string("Creando persona numero ")+nro_persona+".");
                
        //puerta entrar
        int n_puerta_entrada = (rand() % cant_puertas);
        static char puerta_entrada[MAX_DIG_PUERTA];
        sprintf(puerta_entrada,"%d",n_puerta_entrada);
         
        //puerta salir
        int n_puerta_salida = (rand() % cant_puertas);
        static char puerta_salida[MAX_DIG_PUERTA];
        sprintf(puerta_salida,"%d",n_puerta_salida);
         
        //tiempo en museo
        int n_paseando = (rand() % MAX_RAND) + 1;
        static char paseando[MAX_DIG_PUERTA];
        sprintf(paseando,"%d",n_paseando);
        
         
        pid_hijo_persona = fork();
        if (pid_hijo_persona < 0) {
            (Logger::getLogger())->escribir(ERROR,string(" No se pudo crear la persona")+nro_persona+".");
            Logger::destroy();
            exit(1);
        }
        if (pid_hijo_persona == 0) { //(hijo) persona
            execlp(PERSONA_EXE,PERSONA_EXE,puerta_entrada,puerta_salida,paseando,(char*) 0);
            (Logger::getLogger())->escribir(ERROR,std::string(strerror(errno))+string(" No se pudo ejecutar la persona ")+nro_persona+".");
            Logger::destroy();
            exit(1);
        }
        sleep((rand() % 2) + 1); //tiempo entre una persona y otra
    }
    (Logger::getLogger())->escribir(MSJ,string("---Finalizada la creación de personas.---"));
    Logger::destroy();
    return 0;
}

