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
#include <semaphore.h>
#include <commons/collections/queue.h>
#include "entrenador.h"
#include "appeared_pokemon.h"
#include "auxiliar.h"

#define IP "127.0.0.3"
#define PUERTO "37229"

typedef enum{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
	LOCALIZED_POKEMON = 6,
	SUSCRIPTOR = 7,
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

t_log* logger_team;
t_log* logger_prueba;
pthread_t thread;
t_queue* appeared_pokemons;
t_list* objetivo_global;
t_list* especies_requeridas;
t_queue* cola_ready;
int socket_servidor;
sem_t sem_appeared_pokemon;
sem_t sem_entrenadores;
sem_t puede_planificar;
sem_t* puede_ejecutar;
sem_t* llega_mensaje_caught;
t_appeared_pokemon* pokemon_a_atrapar;
u_int32_t id_cola_localized;
u_int32_t id_cola_caught;
u_int32_t id_cola_appeared;

// Funciones Servidor

void* recibir_buffer(int*, int);
void iniciar_servidor(void);
void esperar_cliente(int);
void* recibir_mensaje(int socket_cliente, int* size);
void process_request(int, int);
void serve_client(int*);
u_int32_t recibir_entero(int socket_cliente);

// Funciones Cliente

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* argv[], u_int32_t socket_cliente);
void liberar_conexion(u_int32_t socket_cliente);
void serializar_mensaje(tipo_mensaje tipo, char** argv,u_int32_t socket_cliente, u_int32_t tamanio);
u_int32_t obtener_size(char* argumentos[], tipo_mensaje tipo);
void* generar_stream(char** argumentos, t_paquete* paquete);
void agregar_string(int* offset, char* string, void** stream);
void agregar_entero(int* offset, char* string, void** stream);
void asignar_id_cola_de_mensajes(u_int32_t id_a_asignar, tipo_mensaje tipo);

// Funciones compartidas

tipo_mensaje obtener_tipo_mensaje(char* tipo);
bool quedan_pokemons_por_atrapar();

#endif /* UTILS_H_ */
