/*
 * utils.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_



#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include "entrenador.h"
#include "appeared_pokemon.h"
#include "auxiliar.h"

#define IP "127.0.0.3"
#define PUERTO "37229"

typedef enum{
	APPEARED_POKEMON = 2,
	GET_POKEMON = 5,
}tipo_mensaje;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct{

	t_appeared_pokemon* appeared_pokemon;
	int* socket_cliente;

}t_parametros;

typedef struct
{
	tipo_mensaje codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct
{
	char* nombre;
	int cantidad;
} t_especie;

pthread_t thread;
t_list* appeared_pokemons;
t_list* objetivo_global;
t_list* especies_requeridas;
int socket_servidor;

// Funciones Servidor

void* recibir_buffer(int*, int);
void iniciar_servidor(void);
void esperar_cliente(int);
void* recibir_mensaje(int socket_cliente, int* size);
void process_request(int, int);
void serve_client(int*);
int recibir_entero(int socket_cliente);

// Funciones Cliente

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* argv[], u_int32_t socket_cliente);
void liberar_conexion(u_int32_t socket_cliente);
void serializar_mensaje(tipo_mensaje tipo, char** argv,u_int32_t socket_cliente, u_int32_t tamanio);
u_int32_t obtener_size(char* argumentos[], tipo_mensaje tipo);
void* generar_stream(char** argumentos, t_paquete* paquete);
void agregar_string(int* offset, char* string, void** stream);
void agregar_entero(int* offset, char* string, void** stream);

// Funciones compartidas

tipo_mensaje obtener_tipo_mensaje(char* tipo);
bool objetivo_global_cumplido(t_list* objetivo_global);

#endif /* UTILS_H_ */
