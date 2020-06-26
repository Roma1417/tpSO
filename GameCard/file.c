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

bool esta_abierto(FILE* file) {
	fseek(file, -sizeof(char), SEEK_END);
	char abierto = (char) fgetc(file);
	return (abierto == 'Y');
}

void abrir_file(FILE* file) {
	fseek(file, -sizeof(char), SEEK_END);
	fputc('Y', file);
}

void cerrar_file(FILE* file) {
	fseek(file, -sizeof(char), SEEK_END);
	fputc('N', file);
}

void retroceder_hasta(char caracter_de_paro, FILE** file) {
	char caracter = fgetc(*file);
	while (caracter != caracter_de_paro) {
		printf("Caracter: %c\n", caracter);
		fseek(*file, -2, SEEK_CUR);
		caracter = fgetc(*file);
		sleep(1);
	}
}

void avanzar_hasta(char caracter_de_paro, FILE** file) {
	char caracter = fgetc(*file);
	while (caracter != caracter_de_paro) {
		caracter = fgetc(*file);
	}
}

char* guardar_hasta(char caracter_de_paro, FILE** file) {
	char caracter = fgetc(*file);
	char* cadena_guardada = string_new();
	while (caracter != caracter_de_paro) {
		string_append_with_format(&cadena_guardada, "%c", caracter);
		caracter = fgetc(*file);
	}
	return cadena_guardada;
}

char* guardar_hasta_con_EOF(char caracter_de_paro, FILE** file,
		bool* llego_al_final) {
	char caracter = fgetc(*file);
	char* cadena_guardada = string_new();
	while (caracter != caracter_de_paro && caracter != EOF) {
		string_append_with_format(&cadena_guardada, "%c", caracter);
		caracter = fgetc(*file);
		printf("Caracter: %c\n", caracter);
	}
	printf("Cadena_guardada: %s\n", cadena_guardada);
	(*llego_al_final) = (caracter == EOF);
	if (*llego_al_final)
		fseek(*file, -1, SEEK_CUR);
	return cadena_guardada;
}

void verificar_estado_de_apertura_de_archivo_pokemon(FILE* file) {
	while (esta_abierto(file))
		sleep(config_gamecard->tiempo_de_reintento_operacion);

	abrir_file(file);
}

char* obtener_ultimo_bloque(FILE* file) {
	fseek(file, -2, SEEK_END);
	char bloque = fgetc(file);
	while ((bloque != ',') && (bloque != '[')) {
		fseek(file, -2, SEEK_CUR);
		bloque = fgetc(file);
	}
	char* ultimo_bloque = string_new();
	bloque = fgetc(file);
	while (bloque != ']') {
		string_append_with_format(&ultimo_bloque, "%c", bloque);
		bloque = fgetc(file);
	}
	//printf("Ultimo bloque: %s\n", ultimo_bloque);

	return ultimo_bloque;
}

char* obtener_directorio_block_path() {
	char* path_beta = string_new();
	string_append(&path_beta, "/Blocks");
	char* path = generar_nombre(path_beta);
	return path;
}

char* obtener_bloque_path(char* bloque) {
	char* path = obtener_directorio_block_path();
	string_append_with_format(&path, "/%s.bin", bloque);
	return path;
}

char* obtener_bloque_disponible() {
	bool encontrado;
	uint32_t bloque_disponible;
	for (int i = 0; i < (metadata_general->blocks) && !encontrado; i++) {
		if (bitarray_test_bit(bitmap, i) == 0) {
			bloque_disponible = i + 1;
			printf("Bloque_disponible: %d\n", bloque_disponible);
			encontrado = true;
			bitarray_set_bit(bitmap, i);
		}
	}
	return string_itoa(bloque_disponible);
}

uint32_t file_size(FILE* file) {
	uint32_t contador = 0;
	fseek(file, 0, SEEK_SET);
	char caracter = fgetc(file);
	while (caracter != EOF) {
		contador++;
		caracter = fgetc(file);
	}
	return contador;
}

void guardar_posterior(FILE* file, char** posterior) {
	char caracter = fgetc(file);
	while (caracter != '\n') {
		caracter = fgetc(file);
	}

	caracter = fgetc(file);
	while (caracter != EOF) {
		string_append_with_format(posterior, "%c", caracter);
		caracter = fgetc(file);
	}
}

void actualizar_size(FILE* file, FILE* bloque) {
	char* posterior = string_new();
	fseek(file, 16, SEEK_SET);
	guardar_posterior(file, &posterior);
	//printf("Posterior: %s", posterior);
	fseek(file, 17, SEEK_SET);
	fputs(string_itoa(file_size(bloque)), file);
	fputc('\n', file);
	fputs(posterior, file);
	fseek(file, -string_length(posterior), SEEK_END);
	free(posterior);
}

void actualizar_metadata(FILE* bloque_file, FILE* file, char* ultimo_bloque,
		char* bloque) {
	//Actualizar en Metadata
	char caracter;
	char* bloque_string = string_new();
	string_append(&bloque_string, ultimo_bloque);
	char* posterior = string_new();
	fseek(file, 0, SEEK_SET);
	//actualizar_size(file, bloque_file);
	avanzar_hasta(']', &file);
	guardar_posterior(file, &posterior);
	fseek(file, 0, SEEK_SET);
	caracter = fgetc(file);
	while (caracter != ']') {
		caracter = fgetc(file);
		printf("Caracter_imp: %c\n", caracter);
	}
	/*while (caracter != ',' && caracter != '[' ) {
	 fseek(file, -2, SEEK_CUR);
	 caracter = fgetc(file);
	 //printf("Char: %c\n", caracter);
	 }*/
	fseek(file, -1, SEEK_CUR);
	printf("Bloque: %s\n", bloque);
	//printf("Resultado: %d\n", bloque);
	if (!string_equals_ignore_case(bloque, ""))
		fputc(',', file);
	string_append(&bloque_string, "]\n");
	fputs(bloque_string, file);
	fputs(posterior, file);
	free(posterior);
}

void crear_nuevo_bloque(FILE* file, t_new_pokemon* new_pokemon, char* bloque) {
	//Crear nuevo bloque
	char* ultimo_bloque = obtener_bloque_disponible();
	FILE* bloque_file = fopen(obtener_bloque_path(ultimo_bloque), "w+b");
	fputs(string_itoa(new_pokemon->pos_x), bloque_file);
	fputc('-', bloque_file);
	fputs(string_itoa(new_pokemon->pos_y), bloque_file);
	fputc('=', bloque_file);
	fputs(string_itoa(new_pokemon->cantidad), bloque_file);
	fputc('\n', bloque_file);

	//Actualizar en Metadata
	actualizar_metadata(bloque_file, file, ultimo_bloque, bloque);
	fclose(bloque_file);
}

t_list* obtener_bloques_del_pokemon(FILE* file) {
	fseek(file, 0, SEEK_SET);
	avanzar_hasta('K', &file);
	avanzar_hasta('=', &file);
	char* bloques_string = guardar_hasta('\n', &file);
	t_list* bloques = list_create();
	char* bloque_leido = string_new();
	printf("Bloques: %s\n", bloques_string);
	int i = 0;
	while (bloques_string[i] != '\0') {
		if (bloques_string[i] != ',' && bloques_string[i] != '[' && bloques_string[i] != ']')
			string_append_with_format(&bloque_leido, "%c", bloques_string[i]);
		else if (bloques_string[i] == ',' || bloques_string[i] == ']'){
			list_add(bloques, string_duplicate(bloque_leido));
			free(bloque_leido);
			bloque_leido = string_new();
		}

		i++;
	}
	return bloques;
}

bool posicion_ya_cargada(char* posicion_actual, t_list* posiciones) {
	bool es_la_posicion_buscada(void* parametro) {
		char* posicion = (char*) parametro;
		return string_starts_with(posicion, posicion_actual);
	}
	return list_any_satisfy(posiciones, es_la_posicion_buscada);
}

void actualizar_posicion_ya_cargada(FILE* bloque_file,
		t_new_pokemon* new_pokemon) {
	char caracter;
	char* posterior = string_new();
	uint32_t cantidad_total;
	char* cantidad;
	printf("Llegue aca\n");
	//fseek(bloque_file, 2, SEEK_CUR);
	retroceder_hasta('=', &bloque_file);
	printf("Llegue aca tambien\n");
	cantidad = guardar_hasta('\n', &bloque_file);
	cantidad_total = atoi(cantidad);
	printf("Cantidad_parcial = %s\n", cantidad);
	cantidad_total += new_pokemon->cantidad;
	printf("Cantidad_total = %d\n", cantidad_total);

	fseek(bloque_file, -1, SEEK_CUR);
	guardar_posterior(bloque_file, &posterior);
	printf("Posterior: %s\n", posterior);
	fseek(bloque_file, -strlen(posterior), SEEK_CUR);
	retroceder_hasta('=', &bloque_file);
	fputs(string_itoa(cantidad_total), bloque_file);
	fputc('\n', bloque_file);
	fputs(posterior, bloque_file);
}

bool no_entra_en_bloque(FILE* file, t_new_pokemon* new_pokemon,
		char* ultimo_bloque) {
	char* cadena_a_agregar = string_new();
	string_append_with_format(&cadena_a_agregar, "%s-%s=%s\n",
			string_itoa(new_pokemon->pos_x), string_itoa(new_pokemon->pos_y),
			string_itoa(new_pokemon->cantidad));
	char* bloque_path = obtener_bloque_path(ultimo_bloque);
	FILE* bloque_file = fopen(bloque_path, "r+");
	printf("Block_size: %d\n", metadata_general->block_size);
	printf("File_size: %d\n", file_size(bloque_file));
	printf("String_length: %d\n", string_length(cadena_a_agregar));
	int diferencia_de_sizes = (metadata_general->block_size
			- (file_size(bloque_file) + string_length(cadena_a_agregar)));
	printf("Diferencia_de_sizes: %d\n", diferencia_de_sizes);
	bool entra_en_bloque = (diferencia_de_sizes >= 0);
	printf("Entra_en_bloque: %d\n", entra_en_bloque);
	fclose(bloque_file);
	return !entra_en_bloque;
}

t_list* obtener_posiciones_actuales(FILE* file, t_list* bloques_file,
		t_list* bloques) {
	FILE* bloque_file;
	char* posicion;
	char* auxiliar;
	char caracter;
	bool llego_al_final = false;
	int i = 0;
	t_list* posiciones = list_create();
	//for (int i = 0; i < list_size(bloques); i++) {
		printf("Bloque: %s\n", list_get(bloques, i));
		bloque_file = list_get(bloques_file, i);
		printf("Ward2\n");
		fseek(bloque_file, 1, SEEK_SET);
		while (!feof(bloque_file)) {
			fseek(bloque_file, -1, SEEK_CUR);
			printf("Ward3\n");
			posicion = guardar_hasta_con_EOF('\n', &bloque_file,
					&llego_al_final);
			printf("Ward4\n");
			printf("Llego_al_final: %d\n", llego_al_final);
			if (llego_al_final) {
				i++;
				bloque_file = list_get(bloques_file, i);
				auxiliar = guardar_hasta('\n', &bloque_file);
				string_append_with_format(&posicion, auxiliar);
			}
			string_append(&posicion, "\n");
			list_add(posiciones, posicion);
			fgetc(bloque_file);
		}
		printf("Salí del while\n");
	//}

	return posiciones;
}

uint32_t obtener_cantidad(char* posicion) {
	char* cantidad = string_new();
	char caracter;
	int i = 0;
	caracter = posicion[i];
	while (caracter != '=') {
		i++;
		caracter = posicion[i];
	}
	i++;
	caracter = posicion[i];
	while (caracter != '\0') {
		string_append_with_format(&cantidad, "%c", caracter);
		i++;
		caracter = posicion[i];
	}

	return atoi(cantidad);
}

t_list* obtener_bloques_actuales(FILE* file, t_list* bloques) {
	t_list* bloques_file = list_create();
	char* bloque_path;
	FILE* bloque_file;
	for (int i = 0; i < list_size(bloques); i++) {
		bloque_path = obtener_bloque_path(list_get(bloques, i));
		printf("Bloque_path: %s\n", bloque_path);
		bloque_file = fopen(bloque_path, "r+");
		list_add(bloques_file, bloque_file);
		free(bloque_path);
	}
	return bloques_file;
}

void actualizar_posiciones(FILE* file, t_new_pokemon* new_pokemon) {
	char* ultimo_bloque = obtener_ultimo_bloque(file);
	if (string_equals_ignore_case(ultimo_bloque, "")) { // || no_entra_en_bloque(file, new_pokemon, ultimo_bloque)){
		//Crear nuevo bloque
		crear_nuevo_bloque(file, new_pokemon, ultimo_bloque);
	} else {
		//Actualizar ultimo_bloque (Validar que no supere BLOCK_SIZE)
		printf("Hay bloques\n");
		char* posicion_actual = string_new();
		string_append_with_format(&posicion_actual, "%s-%s=",
				string_itoa(new_pokemon->pos_x),
				string_itoa(new_pokemon->pos_y));
		t_list* bloques = obtener_bloques_del_pokemon(file);
		for(int i=0;i<list_size(bloques); i++) printf("Bloque %d: %s\n", i, list_get(bloques, i));
		t_list* bloques_file = obtener_bloques_actuales(file, bloques);
		printf("Bloques_file_size: %d\n", list_size(bloques_file));
		t_list* posiciones = obtener_posiciones_actuales(file, bloques_file,
				bloques);
		char* bloque;
		FILE* bloque_file;

		for (int i = 0; i < list_size(posiciones); i++) {
			printf("Posicion: %s\n", (char*) list_get(posiciones, i));
		}

		if (posicion_ya_cargada(posicion_actual, posiciones)) {
			printf("Entre al IF\n");
			int indice_del_encontrado = 0;
			bool encontrado = false;
			uint32_t cantidad = 0;
			for (int i = 0; i < list_size(posiciones) && !encontrado; i++) {
				encontrado = string_starts_with(list_get(posiciones, i),posicion_actual);
				if (encontrado)
					indice_del_encontrado = i;
			}
			printf("Indice: %d\n", indice_del_encontrado);
			char* posicion = list_get(posiciones, indice_del_encontrado);
			//printf("Posicion_encontrada: %s\n", posicion);

			cantidad = obtener_cantidad(posicion) + new_pokemon->cantidad;
			char** xey = string_split(posicion, "=");
			free(posicion);
			posicion = xey[0];
			string_append(&posicion, "=");
			string_append(&posicion, string_itoa(cantidad));
			string_append(&posicion, "\n");
			printf("Posicion_cargada: %s\n", posicion);
			list_replace(posiciones, indice_del_encontrado, posicion);
			char* pos;
			int contador = 0;
			for(int i=0; i<list_size(posiciones);i++){
				printf("Posicion %d: %s\n", i, list_get(posiciones, i));
			}

			for (int i = 0; i < list_size(bloques_file); i++) {
				bloque_file = list_get(bloques_file, i);
				fseek(bloque_file, 0, SEEK_SET);
			}

			int k = 0;

			for (int i = 0; i < list_size(posiciones); i++) {
				pos = list_get(posiciones, i);

				for (int j = 0; j < string_length(pos); j++) {
					if (contador < (metadata_general->block_size)) {
						bloque_file = list_get(bloques_file, k);
						fputc(pos[j], bloque_file);
						printf("Puse caracter %c en bloque %d\n", pos[j],
								k + 1);
						contador++;
					} else if (k < (list_size(bloques_file) - 1)) {
						contador = 0;
						k++;
						bloque_file = list_get(bloques_file, k);
						fputc(pos[j], bloque_file);
					} else {
						contador = 0;
						//Crear nuevo bloque y ponerlo ahí
						bloque = obtener_bloque_disponible();
						actualizar_metadata(bloque_file, file, bloque,ultimo_bloque);
						bloque_file = fopen(obtener_bloque_path(bloque),
								"w+b");
						free(ultimo_bloque);
						list_add(bloques_file, bloque_file);
						k++;
						fputc(pos[j], list_get(bloques_file, k));
					}
				}
				//fputc('\n', bloque_file);
			}

			for (int i = 0; i < list_size(bloques_file); i++) {
				bloque_file = list_get(bloques_file, i);
				fclose(bloque_file);
			}
		} else {
			printf("Entre al else\n");
			char* bloque_path = obtener_bloque_path(ultimo_bloque);
			printf("Ultimo_bloque: %s\n", ultimo_bloque);
			bloque_file = fopen(bloque_path, "r+");
			int contador = file_size(bloque_file);
			char* posicion_a_agregar = string_new();
			string_append_with_format(&posicion_a_agregar, "%s-%s=%s\n",
					string_itoa(new_pokemon->pos_x),
					string_itoa(new_pokemon->pos_y),
					string_itoa(new_pokemon->cantidad));
			fseek(bloque_file, 0, SEEK_END);
			for (int j = 0; j < string_length(posicion_a_agregar); j++) {
				printf("Contador: %d\n",contador);
				if (contador < (metadata_general->block_size)) {
					fputc(posicion_a_agregar[j], bloque_file);
					printf("Puse caracter %c\n", posicion_a_agregar[j]);
					contador++;
				} else {
					contador = 0;
					//Crear nuevo bloque y ponerlo ahí
					bloque = obtener_bloque_disponible();
					printf("Bloque_disp: %s\n", bloque);
					actualizar_metadata(bloque_file, file, bloque,
							ultimo_bloque);
					printf("Bloque_path: %s\n", obtener_bloque_path(bloque));
					fclose(bloque_file);
					bloque_file = fopen(obtener_bloque_path(bloque), "w+b");
					//free(bloque);
					list_add(bloques_file, bloque_file);
					fputc(posicion_a_agregar[j], bloque_file);
				}
			}
			//fputc('\n', bloque_file);
			fclose(bloque_file);
		}
	}
}

char* generar_nombre(char* parametro) {
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

void generar_metadata_bin(char* pokemon) {
	char* metadata_bin_path = generar_pokemon_metadata_bin_path(pokemon);
	FILE* metadata_file = fopen(metadata_bin_path, "w+b");
	fputs("DIRECTORY=N\n", metadata_file);
	fputs("SIZE=0\n", metadata_file);
	fputs("BLOCKS=[]\n", metadata_file);
	fputs("OPEN=N", metadata_file);
	fclose(metadata_file);
	free(metadata_bin_path);
}
