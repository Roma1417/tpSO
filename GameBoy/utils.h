#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdbool.h>

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
	SUSCRIPTOR = 7,
	DESCONOCIDO = 8,
}tipo_mensaje;

/*typedef struct{
	u_int32_t nombre_size;
	char* nombre_pokemon;
	u_int32_t pos_x;
	u_int32_t pos_y;
	u_int32_t cantidad;
}t_new_pokemon;

typedef struct{
	u_int32_t nombre_size;
	char* nombre_pokemon;
	u_int32_t pos_x;
	u_int32_t pos_y;
	u_int32_t id_mensaje;
}t_appeared_pokemon;

typedef struct{
	u_int32_t nombre_size;
	char* nombre_pokemon;
	u_int32_t pos_x;
	u_int32_t pos_y;
}t_catch_pokemon;

typedef struct{
	u_int32_t id_mensaje;
	u_int32_t 
}t_caught_pokemon;*/

typedef struct
{
	u_int32_t size;
	void* stream;
} t_buffer;

typedef struct
{
	tipo_mensaje codigo_operacion;
	t_buffer* buffer;
} t_paquete;

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* argv[], u_int32_t socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void liberar_conexion(u_int32_t socket_cliente);
tipo_mensaje obtener_tipo_mensaje(char* tipo);
void serializar_mensaje(tipo_mensaje tipo, char** argv,u_int32_t socket_cliente, u_int32_t tamanio);
u_int32_t obtener_size(char* argumentos[], tipo_mensaje tipo);
void* generar_stream(char** argumentos, t_paquete* paquete);
void agregar_string(int* offset, char* string, void** stream);
void agregar_entero(int* offset, char* string, void** stream);
void validar_argumentos(char** argumentos, int cantidad);

#endif /* UTILS_H_ */
