/*
 * file.h
 *
 *  Created on: 20 jun. 2020
 *      Author: utnso
 */

#ifndef FILE_H_
#define FILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>

typedef struct
{
	char* pokemon;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t cantidad;
} t_new_pokemon;

typedef struct
{
	char* pokemon;
	uint32_t pos_x;
	uint32_t pos_y;
} t_catch_pokemon;

typedef struct {
	uint32_t tiempo_de_reintento_conexion;
	uint32_t tiempo_de_reintento_operacion;
	char* punto_montaje_tallgrass;
	char* ip_broker;
	char* puerto_broker;
}t_config_gamecard;

typedef struct {
	uint32_t block_size;
	uint32_t blocks;
	char* magic_number;
}t_metadata_general;

t_config_gamecard* config_gamecard;
char* archivo_bitmap_path;
char* archivo_metadata_general_path;
t_metadata_general* metadata_general;
t_bitarray* bitmap;


char* generar_pokemon_file_path(char* pokemon);
void verificar_existencia_de_archivo(char* pokemon);
bool esta_abierto(FILE* file);
void abrir_file(FILE* file);
void cerrar_file(FILE* file);
void verificar_estado_de_apertura_de_archivo_pokemon(FILE* file);
char* obtener_ultimo_bloque(FILE* file);
char* obtener_directorio_block_path();
char* obtener_bloque_path(char* bloque);
void actualizar_posiciones(FILE* file, t_new_pokemon* new_pokemon);
char* generar_nombre(char* parametro);
char* generar_pokemon_metadata_bin_path(char* pokemon);
void generar_metadata_bin(char* pokemon);
char* obtener_metadata_general_path();
t_list* obtener_bloques_del_pokemon(FILE* file);
FILE* posicionar_en_inicio(t_list* bloques_file, FILE* bloque_file);
t_list* obtener_bloques_del_pokemon(FILE* file);
t_list* obtener_bloques_actuales(FILE* file, t_list* bloques);
void cerrar_bloques_file(t_list* bloques_file, FILE* bloque_file);
void actualizar_size(FILE* file, t_list* bloques_file);
bool posicion_ya_cargada(char* posicion_actual, t_list* posiciones);
uint32_t obtener_cantidad(char* posicion);
void actualizar_posiciones_ya_cargadas(t_list* posiciones, FILE* bloque_file, t_list* bloques_file, FILE* file, char* ultimo_bloque);
FILE* posicionar_en_inicio(t_list* bloques_file, FILE* bloque_file);
char* cargar_nueva_posicion(uint32_t cantidad, char* posicion);
int obtener_indice_del_encontrado(int* indice_del_encontrado,t_list* posiciones, char* posicion_actual);
t_list* obtener_posiciones_actuales(FILE* file, t_list* bloques_file, t_list* bloques);
char* guardar_hasta(char caracter_de_paro, FILE** file);
char* guardar_hasta_EOF(FILE** file);
void avanzar_hasta(char caracter_de_paro, FILE** file);
void retroceder_hasta(char caracter_de_paro, FILE** file);

#endif /* FILE_H_ */
