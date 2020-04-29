#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>

/*typedef enum
{
	MENSAJE = 1,
}op_code;*/

typedef enum{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
}tipo_mensaje;

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

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* argv[], int socket_cliente, int tamanio);
//char* recibir_mensaje(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void liberar_conexion(int socket_cliente);
tipo_mensaje obtener_tipo_mensaje(char* tipo);
void serializar_mensaje(tipo_mensaje tipo, char** argv,int socket_cliente, int tamanio);
int obtener_size(char* argumentos[], int tamanio);
void* generar_stream(char** argumentos, int tamanio, int size);

#endif /* UTILS_H_ */
