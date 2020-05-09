/*
 * utils.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_



#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include "entrenador.h"

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

	u_int32_t size_pokemon;
	char* pokemon;
	t_posicion* posicion;

} t_appeared_pokemon;

typedef struct{

	t_appeared_pokemon* appeared_pokemon;
	int* socket_cliente;

}t_parametros;

typedef struct
{
	tipo_mensaje codigo_operacion;
	t_buffer* buffer;
} t_paquete;

pthread_t thread;

// Funciones Servidor

void* recibir_buffer(int*, int);
t_appeared_pokemon* iniciar_servidor(void);
void esperar_cliente(int, t_parametros*);
void* recibir_mensaje(int socket_cliente, int* size);
void process_request(int cod_op, t_parametros* parametros);
void serve_client(t_parametros* parametros);
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

#endif /* UTILS_H_ */
