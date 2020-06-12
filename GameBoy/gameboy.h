/*
 * Cliente.h
 *
 *  Created on: 28 feb. 2019
 *      Author: utnso
 */

#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<pthread.h>
#include "utils.h"

pthread_t thread;


t_log* iniciar_logger(void);
t_config* leer_config(void);
void terminar_programa(int, t_log*, t_config*);
char* obtener_key(char* parametro, char* destino);
void obtener_parametro(char ** parametro, char* string_parametro, char* destino, t_config* config);
char** caso_suscriptor(char** argv);
char** caso_caught(char** argv);
void evaluar_suscripcion(char** argv, int conexion);

#endif /* GAMEBOY_H_ */
