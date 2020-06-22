/*
 * file.c
 *
 *  Created on: 20 jun. 2020
 *      Author: utnso
 */
#include "file.h"

char* generar_pokemon_file_path(char* pokemon) {
	char* path_beta = string_new();
	string_append_with_format(&path_beta, "/Files/%s", pokemon);
	char* path = generar_nombre(path_beta);
	return path;
}

void verificar_existencia_de_archivo(char* pokemon) {
	char* path_pokemon_file = generar_pokemon_file_path(pokemon);
	DIR* directorio_pokemon = opendir(path_pokemon_file);
	if (directorio_pokemon == NULL) {
		mkdir(path_pokemon_file, 0777);
		generar_metadata_bin(pokemon);
	}
	free(path_pokemon_file);
	closedir(directorio_pokemon);
}

bool esta_abierto(FILE* file){
	fseek(file, -sizeof(char), SEEK_END);
	char abierto = (char) fgetc(file);
	return (abierto == 'Y');
}

void abrir_file(FILE* file){
	fseek(file, -sizeof(char), SEEK_END);
	fputc('Y', file);
}

void cerrar_file(FILE* file){
	fseek(file, -sizeof(char), SEEK_END);
	fputc('N', file);
}

void verificar_estado_de_apertura_de_archivo_pokemon(FILE* file){
	while(esta_abierto(file))
		sleep(config_gamecard->tiempo_de_reintento_operacion);

	abrir_file(file);
}

char* obtener_ultimo_bloque(FILE* file){
	fseek(file, -2, SEEK_END);
	char bloque = fgetc(file);
	while((bloque != ',') && (bloque != '[')){
		fseek(file, -2, SEEK_CUR);
		bloque = fgetc(file);
	}
	char* ultimo_bloque = string_new();
	bloque = fgetc(file);
	while(bloque != ']'){
		string_append_with_format(&ultimo_bloque, "%c", bloque);
		bloque = fgetc(file);
	}
	//printf("Ultimo bloque: %s\n", ultimo_bloque);

	return ultimo_bloque;
}

char* obtener_directorio_block_path(){
	char* path_beta = string_new();
	string_append(&path_beta, "/Blocks");
	char* path = generar_nombre(path_beta);
	return path;
}

char* obtener_bloque_path(char* bloque){
	char* path = obtener_directorio_block_path();
	string_append_with_format(&path, "/%s.bin", bloque);
	return path;
}

char* obtener_bloque_disponible(){
	bool encontrado;
	uint32_t bloque_disponible;
	for(int i=0; i<(metadata_general->blocks) && !encontrado; i++){
		if(bitarray_test_bit(bitmap, i) == 0){
			bloque_disponible = i+1;
			encontrado = true;
			bitarray_set_bit(bitmap, i);
		}
	}
	return string_itoa(bloque_disponible);
}

uint32_t file_size(FILE* file){
	uint32_t contador = 0;
	fseek(file, 0, SEEK_SET);
	char caracter = fgetc(file);
	while(caracter != EOF){
		contador++;
		caracter = fgetc(file);
	}
	return contador;
}

void crear_nuevo_bloque(FILE* file, t_new_pokemon* new_pokemon) {
	//Crear nuevo bloque
	char* ultimo_bloque = obtener_bloque_disponible();
	FILE* bloque = fopen(obtener_bloque_path(ultimo_bloque), "w+b");
	fputs(string_itoa(new_pokemon->pos_x), bloque);
	fputs("-", bloque);
	fputs(string_itoa(new_pokemon->pos_y), bloque);
	fputs("=", bloque);
	fputs(string_itoa(new_pokemon->cantidad), bloque);
	fputs("\n", bloque);


	//Actualizar en Metadata
	char* posterior = string_new();
	uint32_t longitud_file = file_size(bloque);
	fclose(bloque);
	fseek(file, 16, SEEK_SET);
	char caracter = fgetc(file);
	while(caracter != '\n'){
		caracter = fgetc(file);
	}
	caracter = fgetc(file);
	while(caracter != EOF){
		string_append_with_format(&posterior, "%c", caracter);
		caracter = fgetc(file);
	}
	printf("Posterior: %s", posterior);
	fseek(file, 17, SEEK_SET);
	//char caracter = fgetc(file);
	//printf("char: %c\n", caracter);
	//printf("Ward1\n");
	fputs(string_itoa(longitud_file), file);
	fputc('\n', file);
	fputs(posterior, file);
	fseek(file, -string_length(posterior), SEEK_END);
	free(posterior);
	posterior = string_new();

	caracter = fgetc(file);
	while(caracter != '\n'){
		printf("Char: %c\n", caracter);
		caracter = fgetc(file);
		sleep(1);
	}
	caracter = fgetc(file);
	while(caracter != EOF){
	printf("Caracter: %c\n", caracter);
	string_append_with_format(&posterior, "%c", caracter);
	caracter = fgetc(file);
	sleep(1);
	}

	fseek(file, 0, SEEK_SET);
	caracter = fgetc(file);
	while(caracter != '['){
		caracter = fgetc(file);
	}

	string_append(&ultimo_bloque, "]\n");
	fputs(ultimo_bloque, file);
	fputs(posterior, file);
	free(posterior);
}

void actualizar_posiciones(FILE* file, t_new_pokemon* new_pokemon){
	char* ultimo_bloque = obtener_ultimo_bloque(file);
	if(string_equals_ignore_case(ultimo_bloque,"")){
		//Crear nuevo bloque

		crear_nuevo_bloque(file, new_pokemon);
	}
	else{
		//Actualizar ultimo_bloque (Validar que no supere BLOCK_SIZE)
		char* bloque_path = obtener_bloque_path(ultimo_bloque);
		FILE* bloque_file = fopen(bloque_path, "r+");
		fclose(bloque_file);
	}
}

char* generar_nombre(char* parametro){
	char* nombre = string_new();
	string_append(&nombre, config_gamecard->punto_montaje_tallgrass);
	string_append(&nombre, parametro);
	return nombre;
}

char* generar_pokemon_metadata_bin_path(char* pokemon) {
	char* path = generar_pokemon_file_path(pokemon);
	string_append(&path, "/Metadata.bin");
	return path;
}

void generar_metadata_bin(char* pokemon){
	char* metadata_bin_path = generar_pokemon_metadata_bin_path(pokemon);
	FILE* metadata_file = fopen(metadata_bin_path, "w+b");
	fputs("DIRECTORY=N\n",metadata_file);
	fputs("SIZE=0\n",metadata_file);
	fputs("BLOCKS=[]\n",metadata_file);
	fputs("OPEN=N",metadata_file);
	fclose(metadata_file);
	free(metadata_bin_path);
}
