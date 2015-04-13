/* 
 * File:   Logger.cpp
 * Author: knoppix
 * 
 * Created on March 27, 2015, 8:54 PM
 */

#include "Logger.h"

Logger* Logger::instancia = NULL;

Logger::Logger(std::string path,std::string proceso){
    log_file = open(path.c_str(),O_RDWR|O_APPEND|O_CREAT,0777);
    this->proceso = proceso;
}

Logger::~Logger() {
    close(log_file);
}

Logger* Logger::getLogger(){
    if (instancia == NULL ) {
        std::string msj = "Logger no iniciado";
        write(STDERR_FILENO,msj.c_str(),sizeof(char)*msj.size());
    }
    return instancia;
}

void Logger::init(std::string path,std::string proceso){
    instancia = new Logger(path,proceso);
}

void Logger::destroy(){
    delete instancia;
}

void Logger::escribir(int type,std::string mensaje){ //TODO timestamp y distintos tipos de escribir (error, msj)
    
    char date[20];
    std::string msj;
    
    int res = flock(log_file,LOCK_EX);
    if (res != 0) {
        perror("Logger: No se pudo tomar el lock.");
        return;
    }
    
    time_t timer = time(NULL);
    strftime(date, sizeof(date), "%Y-%m-%d %T ", gmtime(&timer));
    
    struct timeval detail_time;
    gettimeofday(&detail_time,NULL);
    std::stringstream tiempo;
    std::stringstream us;
    us << std::setfill('0') << std::setw(4) << detail_time.tv_usec%1000;
    tiempo << date << detail_time.tv_usec /1000 <<"ms "<< us.str() << "us";
    
    if (type == ERROR) {
        msj = "-ERROR-";
    } else if (type == MSJ) {
        msj = "-MESSAGE-";
    }
    msj = proceso + " " + msj + " " + tiempo.str() + ": " + mensaje + "\n";
    write(log_file,msj.c_str(),sizeof(char)*msj.size());
    write(STDOUT_FILENO,msj.c_str(),sizeof(char)*msj.size());
    
    res = flock(log_file,LOCK_UN);
    if (res != 0) {
        perror("Logger: No se pudo devolver el lock.");
        return;
    }
    return;    
}

