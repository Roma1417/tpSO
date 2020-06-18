/*
 * utils.h
 *
 *  Created on: 7 jun. 2020
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <sys/stat.h>
#include <commons/config.h>
#include <commons/log.h>
#include <dirent.h>

#define IP "127.0.0.2"
#define PUERTO "37228"

typedef enum{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
	LOCALIZED_POKEMON = 6,
	SUSCRIPTOR = 7,
	CONFIRMAR = 8,
}tipo_mensaje;

typedef struct {
	uint32_t tiempo_de_reintento_conexion;
	uint32_t tiempo_de_reintento_operacion;
	char* punto_montaje_tallgrass;
	char* ip_broker;
	char* puerto_broker;

}t_config_gamecard;

pthread_t thread;
u_int32_t id_cola_get;
u_int32_t id_cola_new;
u_int32_t id_cola_catch;
//t_list* archivos_creados;
t_config_gamecard* config_gamecard;
FILE * archivo_metadata;
FILE * archivo_bitmap;
t_log* logger_gamecard;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	tipo_mensaje codigo_operacion;
	t_buffer* buffer;
} t_paquete;

tipo_mensaje obtener_tipo_mensaje(char* tipo);
char* obtener_tipo_mensaje_string(tipo_mensaje tipo);
char* recibir_cadena(int socket_cliente, u_int32_t* size);
void iniciar_servidor(void);
void esperar_cliente(int socket_servidor);
void serve_client(int* socket);
void process_request(int cod_op, int cliente_fd);
u_int32_t recibir_entero(int socket_cliente);
void* serializar_paquete(t_paquete* paquete, u_int32_t *bytes);
int crear_conexion(char *ip, char* puerto);
void enviar_mensaje(char* argv[], u_int32_t socket_cliente);
void agregar_string(int* offset, char* string, void** stream);
void agregar_entero(int* offset, char* string, void** stream);
void* generar_stream(char** argumentos, t_paquete* paquete);
u_int32_t obtener_size(char* argumentos[], tipo_mensaje tipo);
void liberar_conexion(u_int32_t socket_cliente);
void asignar_id_cola_de_mensajes(u_int32_t id_a_asignar, tipo_mensaje tipo);
void recibir_mensaje(int* socket);
bool list_elem(char* elemento, t_list* lista);
char* generar_nombre(char* parametro);
void generar_metadata_bin(char* path);


#endif /* UTILS_H_ */
