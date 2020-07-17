/*p
 * team.c
 *
 *  Created on: 28 abr. 2020
 *      Author: utnso
 */
#include "team.h"

/**
 * @NAME: iniciar_logger
 * @DESC: Crea y devuelve un puntero a una estructura t_log,
 * 		 que sirve como log del proceso Team.
 */
t_log* iniciar_logger(char* path) {

	t_log* logger = log_create(path, "team", true, LOG_LEVEL_INFO);
	if (logger == NULL) {
		printf("No pude crear el logger\n");
		exit(1);
	}
	return logger;

}

/**
 * @NAME: leer_config
 * @DESC: Crea y devuelve un puntero a una estructura t_config.
 */
t_config* leer_config(void) {
	t_config* config = config_create("./teamFinal1.config");
	return config;
}

/**
 * @NAME: construir_config_team
 * @DESC: Crea y devuelve un puntero a una estructura t_config_team,
 * 		 donde sus valores internos corresponden a los guardados en
 * 		 el archivo del team de tipo config.
 */
t_config_team* construir_config_team(t_config* config) {

	t_config_team* config_team = malloc(sizeof(t_config_team));

	config_team->puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	config_team->algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	config_team->ip_broker = config_get_string_value(config, "IP_BROKER");
	config_team->log_file = config_get_string_value(config, "LOG_FILE");

	config_team->tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");

	config_team->quantum = config_get_int_value(config, "QUANTUM");
	config_team->retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	config_team->estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");

	config_team->alpha = config_get_double_value(config, "ALPHA");
	// alpha debe ser float? o double?
	// si es float -> 32bits
	// si es double -> 64bits

	//config_team->objetivos_entrenadores = pasar_a_lista_de_pokemon(config, "OBJETIVOS_ENTRENADORES");
	//config_team->pokemon_entrenadores = pasar_a_lista_de_pokemon(config, "POKEMON_ENTRENADORES");
	//config_team->posiciones_entrenadores = pasar_a_lista_de_posiciones(config, "POSICIONES_ENTRENADORES");

	if (strcmp(config_get_string_value(config, "OBJETIVOS_ENTRENADORES"), "[]") != 0) config_team->objetivos_entrenadores = pasar_a_lista_de_pokemon(config, "OBJETIVOS_ENTRENADORES");
	else config_team->objetivos_entrenadores = list_create();

	if (strcmp(config_get_string_value(config, "POKEMON_ENTRENADORES"), "[]") != 0) config_team->pokemon_entrenadores = pasar_a_lista_de_pokemon(config, "POKEMON_ENTRENADORES");
	else config_team->pokemon_entrenadores = list_create();

	if (strcmp(config_get_string_value(config, "POSICIONES_ENTRENADORES"), "[]") != 0) config_team->posiciones_entrenadores = pasar_a_lista_de_posiciones(config, "POSICIONES_ENTRENADORES");
	else config_team->posiciones_entrenadores = list_create();

	u_int32_t cantidad_faltante = list_size(config_team->posiciones_entrenadores) - list_size(config_team->pokemon_entrenadores);
	if (cantidad_faltante > 0){
		for (int i = 0; i < cantidad_faltante; i++){
			list_add(config_team->pokemon_entrenadores, list_create());
		}
	}

	cantidad_faltante = list_size(config_team->posiciones_entrenadores) - list_size(config_team->objetivos_entrenadores);
	if (cantidad_faltante > 0){
		for (int i = 0; i < cantidad_faltante; i++){
			list_add(config_team->objetivos_entrenadores, list_create());
		}
	}

	return config_team;
}

void agregar_especie_requerida(char* pokemon) {
	t_especie* especie;
	for (int i = 0; i < list_size(especies_requeridas); i++) {
		especie = list_get(especies_requeridas, i);
		if (string_equals_ignore_case(especie->nombre, pokemon)) (especie->cantidad)++;

	}
}

//Codigo de prueba
void* ejecutar_entrenador_FIFO(void* parametro) {
	t_entrenador* entrenador = parametro;

	while (puede_seguir_atrapando(entrenador)) {
		sem_wait(&(puede_ejecutar[entrenador->indice]));
		t_planificado* planificado = planificado_create(entrenador, pokemon_a_atrapar);
		sem_post(&sem_planificado_create);
		int distance = distancia(entrenador->posicion, pokemon_a_atrapar->posicion);
		int ciclos = distance;
		entrenador->ciclos_cpu += ciclos;
		entrenador->rafaga = ciclos;

		sem_wait(&(mutex_ciclos_cpu_totales));
		ciclos_cpu_totales += ciclos;
		sem_post(&(mutex_ciclos_cpu_totales));

		mover_de_posicion(entrenador->posicion, pokemon_a_atrapar->posicion, config_team);
		log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);
		int32_t conexion_catch = enviar_catch_pokemon(entrenador, pokemon_a_atrapar);

		//appeared_pokemon_destroy(pokemon_a_atrapar);

		cambiar_estado(entrenador, BLOCK);
		sem_post(&puede_planificar);

		//printf("Pokemon_a_atrapar de entrenador %d: %s\n", entrenador->indice, pokemon_a_atrapar->pokemon);

		t_appeared_pokemon* auxiliar = planificado->pokemon;
		if (conexion_catch == 0) // Karen, esto es culpa de Ale
			sem_wait(&(llega_mensaje_caught[entrenador->indice]));
		else entrenador->resultado_caught = 1;

		if (entrenador->resultado_caught) {
			cambiar_estado(entrenador, READY);
			queue_push(cola_ready, planificado);
			sem_wait(&(puede_ejecutar[entrenador->indice]));
			atrapar(entrenador, auxiliar);
			log_info(logger_team, "El entrenador %d atrapo a %s en la posicion (%d,%d)", entrenador->indice, auxiliar->pokemon, entrenador->posicion->x, entrenador->posicion->y);
			actualizar_objetivo_global();
			entrenadores_deadlock = filtrar_entrenadores_con_objetivos(entrenadores_deadlock);
		}
		else {
			agregar_especie_requerida(auxiliar->pokemon);
		}
		free(planificado);
		appeared_pokemon_destroy2(auxiliar);
		if (puede_seguir_atrapando(entrenador)) cambiar_condicion_ready(entrenador);
		sem_post(&puede_planificar);
		sem_post(&sem_entrenadores);
	}

	sem_post(&(termino_de_capturar[entrenador->indice]));

	return EXIT_SUCCESS;
}

void* ejecutar_entrenador_RR(void* parametro) {
	t_entrenador* entrenador = parametro;
	int quantum_acumulado = 0;
	int quantum = config_team->quantum;

	while (puede_seguir_atrapando(entrenador)) {
		sem_wait(&(puede_ejecutar[entrenador->indice]));
		t_planificado* planificado = planificado_create(entrenador, pokemon_a_atrapar);
		sem_post(&sem_planificado_create);

		int distance = distancia(entrenador->posicion, pokemon_a_atrapar->posicion);
		printf("POSICION ENTRENADOR: %d %d----------------------\n", entrenador->posicion->x, entrenador->posicion->y);
		printf("POSICION POKEMON: %d %d----------------------\n", pokemon_a_atrapar->posicion->x, pokemon_a_atrapar->posicion->y);
		int ciclos = distance;

		sem_wait(&(mutex_ciclos_cpu_totales));
		printf("Ciclos de ejecutar: %d-------------\n", ciclos_cpu_totales);
		ciclos_cpu_totales += ciclos;
		printf("Ciclos de ejecutar: %d-------------\n", ciclos_cpu_totales);
		sem_post(&(mutex_ciclos_cpu_totales));

		entrenador->rafaga = ciclos;
		entrenador->ciclos_cpu += ciclos;
		log_info(logger_team, "La cantidad de ciclos de CPU necesarios del entrenador %d es: %d", entrenador->indice, distance);

		for (int i = 0; i < distance; i++) {
			u_int32_t distancia_x = distancia_en_x(entrenador->posicion, pokemon_a_atrapar->posicion);
			for (int i = 0; i < distancia_x; i++) {
				if (esta_mas_a_la_derecha(pokemon_a_atrapar->posicion, entrenador->posicion)) mover_a_la_derecha(entrenador->posicion);
				else mover_a_la_izquierda(entrenador->posicion);

				sleep(config_team->retardo_ciclo_cpu);
				quantum_acumulado++;
				entrenador->rafaga--;

				if (quantum_acumulado == quantum) {
					log_info(logger_team, "El entrenador %d vuelve al fin de la cola de ready por fin de quantum", entrenador->indice);
					cambiar_estado(entrenador, READY);
					queue_push(cola_ready, planificado);
					sem_post(&(puede_planificar));
					sem_wait(&puede_ejecutar[entrenador->indice]);
					quantum_acumulado = 0;
				}
			}
			u_int32_t distancia_y = distancia_en_y(entrenador->posicion, pokemon_a_atrapar->posicion);
			for (int j = 0; j < distancia_y; j++) {
				if (esta_mas_arriba(pokemon_a_atrapar->posicion, entrenador->posicion)) mover_hacia_arriba(entrenador->posicion);
				else mover_hacia_abajo(entrenador->posicion);

				sleep(config_team->retardo_ciclo_cpu);
				quantum_acumulado++;
				entrenador->rafaga--;

				if (quantum_acumulado == quantum) {
					log_info(logger_team, "El entrenador %d vuelve al fin de la cola de ready por fin de quantum", entrenador->indice);
					cambiar_estado(entrenador, READY);
					queue_push(cola_ready, planificado);
					sem_post(&(puede_planificar));
					sem_wait(&puede_ejecutar[entrenador->indice]);
					quantum_acumulado = 0;
				}
			}

		}

		log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);
		int32_t conexion_catch = enviar_catch_pokemon(entrenador, pokemon_a_atrapar);
		cambiar_estado(entrenador, BLOCK);
		sem_post(&puede_planificar);

		t_appeared_pokemon* auxiliar = planificado->pokemon;
		if (conexion_catch == 0) // Karen, esto es culpa de Ale
			sem_wait(&(llega_mensaje_caught[entrenador->indice]));
		else entrenador->resultado_caught = 1;

		if (entrenador->resultado_caught) {
			cambiar_estado(entrenador, READY);
			queue_push(cola_ready, planificado);
			sem_wait(&(puede_ejecutar[entrenador->indice]));
			atrapar(entrenador, auxiliar);
			entrenador->rafaga--;
			log_info(logger_team, "El entrenador %d atrapo a %s en la posicion (%d,%d)", entrenador->indice, auxiliar->pokemon, entrenador->posicion->x, entrenador->posicion->y);
			actualizar_objetivo_global();
			printf("WARD YA ACTUALICE EL OBJETIVO GLOBAL----------------------\n");
			entrenadores_deadlock = filtrar_entrenadores_con_objetivos(entrenadores_deadlock);
		}
		else {
			agregar_especie_requerida(auxiliar->pokemon);
		}
		free(planificado);
		appeared_pokemon_destroy2(auxiliar);
		if (puede_seguir_atrapando(entrenador)){
			cambiar_condicion_ready(entrenador);
			sem_post(&sem_entrenadores);
		}
		sem_post(&puede_planificar);
	}

	// wait semaforo todos

	/* Implementar un nuevo hilo

	 while(!cumplio_su_objetivo(entrenador)){
	 sem_wait(&(puede_ejecutar[entrenador->indice]));

	 }*/

	sem_post(&(termino_de_capturar[entrenador->indice]));

	return EXIT_SUCCESS;
}

void verificar_llegada_de_entrenador(t_planificado* planificado_actual, u_int32_t* tamanio_antes) {
	sem_wait(&mutex_largo_lista_ready);

	if ((*tamanio_antes) != largo_lista_ready) {
		sem_post(&mutex_largo_lista_ready);
		t_planificado* planificado = list_get(lista_ready, list_size(lista_ready) - 1);
		float estimacion_restante_entrenador = planificado->entrenador->estimacion_restante;

		if (estimacion_restante_entrenador < planificado_actual->entrenador->estimacion_restante) {
			log_info(logger_team, "El entrenador %d se desaloja para poner en ejecucion al entrenador %d", planificado_actual->entrenador->indice, planificado->entrenador->indice);
			cambiar_estado(planificado_actual->entrenador, READY);
			cambiar_estado(planificado->entrenador, EXEC);
			t_planificado* planificado = elegir_proximo_a_ejecutar_SJFCD();
			pokemon_a_atrapar = planificado->pokemon;
			list_remove(lista_ready, list_size(lista_ready) - 1);
			list_add(lista_ready, planificado_actual);
			sem_post(&(puede_ejecutar[planificado->entrenador->indice]));
			sem_post(&puede_ser_pusheado); // Ale cree que si
			sem_wait(&(puede_ejecutar[planificado_actual->entrenador->indice]));
			*tamanio_antes = list_size(lista_ready);
		}
		else {
			sem_post(&puede_ser_pusheado);
			*tamanio_antes = list_size(lista_ready);
		}
	}
	else {
		sem_post(&mutex_largo_lista_ready);
		sem_post(&puede_ser_pusheado);
	}
}

void* ejecutar_entrenador_SJFCD(void* parametro) {
	t_entrenador* entrenador = parametro;

	while (puede_seguir_atrapando(entrenador)) {
		sem_wait(&(puede_ejecutar[entrenador->indice]));
		t_planificado* planificado = planificado_create(entrenador, pokemon_a_atrapar);
		t_appeared_pokemon* auxiliar = planificado->pokemon;
		sem_post(&sem_planificado_create);
		int distance = distancia(entrenador->posicion, pokemon_a_atrapar->posicion);
		int ciclos = distance;

		sem_wait(&(mutex_ciclos_cpu_totales));
		ciclos_cpu_totales += ciclos;
		sem_post(&(mutex_ciclos_cpu_totales));

		u_int32_t tamanio_antes_de_ejecutar = list_size(lista_ready);

		entrenador->rafaga = ciclos;
		entrenador->ciclos_cpu += ciclos;
		log_info(logger_team, "La cantidad de ciclos de CPU necesarios del entrenador %d es: %d", entrenador->indice, distance);
		for (int i = 0; i < distance; i++) {
			u_int32_t distancia_x = distancia_en_x(entrenador->posicion, pokemon_a_atrapar->posicion);
			for (int i = 0; i < distancia_x; i++) {
				if (esta_mas_a_la_derecha(pokemon_a_atrapar->posicion, entrenador->posicion)) mover_a_la_derecha(entrenador->posicion);
				else mover_a_la_izquierda(entrenador->posicion);

				sleep(config_team->retardo_ciclo_cpu);

				entrenador->rafaga--;
				entrenador->estimacion_restante--;
				verificar_llegada_de_entrenador(planificado, &tamanio_antes_de_ejecutar);
			}

			u_int32_t distancia_y = distancia_en_y(entrenador->posicion, pokemon_a_atrapar->posicion);
			for (int j = 0; j < distancia_y; j++) {
				if (esta_mas_arriba(pokemon_a_atrapar->posicion, entrenador->posicion)) mover_hacia_arriba(entrenador->posicion);
				else mover_hacia_abajo(entrenador->posicion);

				sleep(config_team->retardo_ciclo_cpu);

				entrenador->rafaga--;
				entrenador->estimacion_restante--;
				verificar_llegada_de_entrenador(planificado, &tamanio_antes_de_ejecutar);
			}

		}

		modificar_estimacion_y_rafaga(entrenador, distance);
		entrenador->estimacion_restante = entrenador->estimacion;

		log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);

		int32_t conexion_catch = enviar_catch_pokemon(entrenador, pokemon_a_atrapar);

		cambiar_estado(entrenador, BLOCK);
		sem_post(&puede_planificar);
		//planificado = planificado_create(entrenador, pokemon_a_atrapar);

		if (conexion_catch == 0) // Karen, esto es culpa de Ale
		sem_wait(&(llega_mensaje_caught[entrenador->indice]));
		else entrenador->resultado_caught = 1;

		if (entrenador->resultado_caught) {
			cambiar_estado(entrenador, READY);
			t_appeared_pokemon* auxiliar = planificado->pokemon;
			list_add(lista_ready, planificado);
			sem_wait(&(puede_ejecutar[entrenador->indice]));

			// Comentar si en esta parte hay que calcular la estimacion para
			// rafaga CPU por atrapar = 1
			modificar_estimacion_y_rafaga(entrenador, 1);
			entrenador->estimacion_restante = entrenador->estimacion;

			atrapar(entrenador, auxiliar);
			log_info(logger_team, "El entrenador %d atrapo a %s en la posicion (%d,%d)", entrenador->indice, auxiliar->pokemon, entrenador->posicion->x, entrenador->posicion->y);
			actualizar_objetivo_global();
			entrenadores_deadlock = filtrar_entrenadores_con_objetivos(entrenadores_deadlock);
		}
		else {
			agregar_especie_requerida(auxiliar->pokemon);
		}
		free(planificado);
		appeared_pokemon_destroy2(auxiliar);
		if (puede_seguir_atrapando(entrenador)) cambiar_condicion_ready(entrenador);
		sem_post(&puede_planificar);
		sem_post(&sem_entrenadores);
	}

	sem_post(&(termino_de_capturar[entrenador->indice]));

	printf("WARD TEXAS 1 --------- \n");

	return EXIT_SUCCESS;
}

void* ejecutar_entrenador_SJF(void* parametro) {
	t_entrenador* entrenador = parametro;

	while (puede_seguir_atrapando(entrenador)) {
		sem_wait(&(puede_ejecutar[entrenador->indice]));

		t_planificado* planificado = planificado_create(entrenador, pokemon_a_atrapar);
		sem_post(&sem_planificado_create);

		u_int32_t rafaga = distancia(entrenador->posicion, pokemon_a_atrapar->posicion);

		sem_wait(&(mutex_ciclos_cpu_totales));
		ciclos_cpu_totales += rafaga;
		sem_post(&(mutex_ciclos_cpu_totales));

		entrenador->ciclos_cpu += rafaga;

		mover_de_posicion(entrenador->posicion, pokemon_a_atrapar->posicion, config_team);

		modificar_estimacion_y_rafaga(entrenador, rafaga);

		log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);

		int32_t conexion_catch = enviar_catch_pokemon(entrenador, pokemon_a_atrapar);

		cambiar_estado(entrenador, BLOCK);
		sem_post(&puede_planificar);

		t_appeared_pokemon* auxiliar = planificado->pokemon;
		if (conexion_catch == 0) // Karen, esto es culpa de Ale
		sem_wait(&(llega_mensaje_caught[entrenador->indice]));
		else entrenador->resultado_caught = 1;

		if (entrenador->resultado_caught) {
			cambiar_estado(entrenador, READY);
			t_appeared_pokemon* auxiliar = planificado->pokemon;
			list_add(lista_ready, planificado);
			sem_wait(&(puede_ejecutar[entrenador->indice]));

			// Comentar si en esta parte hay que calcular la estimacion para
			// rafaga CPU por atrapar = 1
			modificar_estimacion_y_rafaga(entrenador, 1);

			atrapar(entrenador, auxiliar);
			log_info(logger_team, "El entrenador %d atrapo a %s en la posicion (%d,%d)", entrenador->indice, auxiliar->pokemon, entrenador->posicion->x, entrenador->posicion->y);
			actualizar_objetivo_global();
			entrenadores_deadlock = filtrar_entrenadores_con_objetivos(entrenadores_deadlock);
		}
		else {
			agregar_especie_requerida(auxiliar->pokemon);
		}
		free(planificado);
		appeared_pokemon_destroy2(auxiliar);
		if (puede_seguir_atrapando(entrenador)) cambiar_condicion_ready(entrenador);
		sem_post(&puede_planificar);
		sem_post(&sem_entrenadores);
	}

	sem_post(&(termino_de_capturar[entrenador->indice]));


	return EXIT_SUCCESS;
}

int32_t enviar_catch_pokemon(t_entrenador* entrenador, t_appeared_pokemon* pokemon) {

	int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);

	if (conexion < 0) return conexion;

	char** mensaje = malloc(sizeof(char*) * 5);
	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "CATCH_POKEMON");

	mensaje[2] = pokemon->pokemon;
	mensaje[3] = string_itoa(pokemon->posicion->x);
	mensaje[4] = string_itoa(pokemon->posicion->y);

	enviar_mensaje(mensaje, conexion);
	for(int i=0;i<5;i++) if (i != 2) free(mensaje[i]);
	free(mensaje);
	asignar_id_caught(entrenador, conexion);

	liberar_conexion(conexion);

	entrenador->ciclos_cpu += ENVIAR_MENSAJE;

	return 0;
}

/**
 * @NAME:
 * @DESC: Crea y devuelve un puntero a una t_list con referencias a
 *        estructuras t_entrenador, inicializados con los valores guardados
 *        en una estructura t_config_team pasada por parametro.
 */
t_list* crear_entrenadores(t_config_team* config_team) {

	t_list* entrenadores = list_create();

	t_list* objetivos_entrenadores = config_team->objetivos_entrenadores;
	t_list* pokemon_entrenadores = config_team->pokemon_entrenadores;
	t_list* posiciones_entrenadores = config_team->posiciones_entrenadores;
	algoritmo_planificacion algoritmo = get_algoritmo_planificacion(config_team);

	printf("El algoritmo de planificacion es: %d\n", algoritmo);

	for (int i = 0; i < list_size(objetivos_entrenadores); i++) {
		t_list* objetivo = list_get(objetivos_entrenadores, i);
		t_list* pokemon_obtenidos;
		if (!list_is_empty(pokemon_entrenadores)) pokemon_obtenidos = list_get(pokemon_entrenadores, i);
		else pokemon_obtenidos = list_create();
		t_posicion* posicion = list_get(posiciones_entrenadores, i);

		t_entrenador* entrenador = entrenador_create(posicion, pokemon_obtenidos, objetivo, i, config_team->estimacion_inicial);
		pthread_t hilo;

		switch (algoritmo) {
			case (FIFO):
				pthread_create(&hilo, NULL, ejecutar_entrenador_FIFO, (void*) entrenador);
				break;
			case (RR):
				pthread_create(&hilo, NULL, ejecutar_entrenador_RR, (void*) entrenador);
				break;
			case (SJF):
				pthread_create(&hilo, NULL, ejecutar_entrenador_SJF, (void*) entrenador);
				break;
			case (SJFCD):
				pthread_create(&hilo, NULL, ejecutar_entrenador_SJFCD, (void*) entrenador);
				break;
		}

		//pthread_create(&hilo, NULL, ejecutar_entrenador, (void*) entrenador);
		//pthread_create(&hilo, NULL, ejecutar_entrenador_SJF, (void*) entrenador);
		//pthread_create(&hilo, NULL, ejecutar_entrenador_RR, (void*) entrenador);

		set_hilo(entrenador, hilo);
		list_add(entrenadores, entrenador);
		list_add(entrenadores_deadlock, entrenador);
	}

	return entrenadores;
}

/*
 * @NAME: enviar_get_pokemon
 * @DESC: Establece conexion con el broker y envia un mensaje GET_POKEMON,
 * 		 para un pokemon pasado por parametro.
 * 		 Recordar -> Pokemon requiere casteo a (char*).
 */
void* enviar_get_pokemon(void* parametro) {
	char* pokemon = parametro;
	char** mensaje = malloc(sizeof(char*) * 3);
	int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);

	if (conexion > 0) {
		mensaje[0] = string_new();
		string_append(&(mensaje[0]), "BROKER");

		mensaje[1] = string_new();
		string_append(&(mensaje[1]), "GET_POKEMON");

		mensaje[2] = pokemon;
		enviar_mensaje(mensaje, conexion);
		for(int i = 0; i < 2; i++) free(mensaje[i]);
		liberar_conexion(conexion);
	}

	free(mensaje);

	return EXIT_SUCCESS;
}

/*
 * @NAME: enviar_mensajes_get_pokemon
 * @DESC: Envia un mensaje get_pokemon al broker por cada especie requerida.
 */
void enviar_mensajes_get_pokemon() {

	pthread_t get_pokemon[list_size(especies_requeridas)];
	t_especie* especie;
	for (int i = 0; i < list_size(especies_requeridas); i++) {
		especie = list_get(especies_requeridas, i);
		pthread_create(&(get_pokemon[i]), NULL, enviar_get_pokemon, (void*) especie->nombre);
		pthread_join(get_pokemon[i], NULL);
	}
}

/*
 * @NAME: enreadyar_al_mas_cercano (FUE IDEA DE JOSI)
 * @DESC: Dados una lista de entrenadores y un appeared_pokemon, pone
 * 		  en estado READY al entrenador mas cercano a ese pokemon.
 */
void enreadyar_al_mas_cercano(t_list* entrenadores, t_appeared_pokemon* appeared_pokemon) {
	t_entrenador* mas_cercano = list_find(entrenadores, puede_ser_planificado);
	int distancia_minima = distancia(mas_cercano->posicion, appeared_pokemon->posicion);

	for (int i = 1; i < list_size(entrenadores); i++) {
		t_entrenador* entrenador_actual = list_get(entrenadores, i);
		if (puede_ser_planificado(entrenador_actual) && (distancia(entrenador_actual->posicion, appeared_pokemon->posicion) < distancia_minima)) {
			mas_cercano = entrenador_actual;
			distancia_minima = distancia(entrenador_actual->posicion, appeared_pokemon->posicion);
		}
	}
	cambiar_estado(mas_cercano, READY);
	cambiar_condicion_ready(mas_cercano);
	t_planificado* planificado = planificado_create(mas_cercano, appeared_pokemon);
	enreadyar(planificado);
	queue_push(cola_ready, planificado);
}

void enreadyar_al_mas_cercano_SJF(t_list* entrenadores, t_appeared_pokemon* appeared_pokemon) {
	t_entrenador* mas_cercano = list_find(entrenadores, puede_ser_planificado);
	int distancia_minima = distancia(mas_cercano->posicion, appeared_pokemon->posicion);

	for (int i = 1; i < list_size(entrenadores); i++) {

		t_entrenador* entrenador_actual = list_get(entrenadores, i);
		if (puede_ser_planificado(entrenador_actual) && (distancia(entrenador_actual->posicion, appeared_pokemon->posicion) < distancia_minima)) {
			mas_cercano = entrenador_actual;
			distancia_minima = distancia(entrenador_actual->posicion, appeared_pokemon->posicion);
		}
	}
	cambiar_estado(mas_cercano, READY);
	cambiar_condicion_ready(mas_cercano);
	t_planificado* planificado = planificado_create(mas_cercano, appeared_pokemon);
	enreadyar(planificado);
	list_add(lista_ready, planificado);
}

void enreadyar_al_mas_cercano_SJFCD(t_list* entrenadores, t_appeared_pokemon* appeared_pokemon) {
	t_entrenador* mas_cercano = list_find(entrenadores, puede_ser_planificado);
	int distancia_minima = distancia(mas_cercano->posicion, appeared_pokemon->posicion);
	for (int i = 1; i < list_size(entrenadores); i++) {

		t_entrenador* entrenador_actual = list_get(entrenadores, i);
		if (puede_ser_planificado(entrenador_actual) && (distancia(entrenador_actual->posicion, appeared_pokemon->posicion) < distancia_minima)) {
			mas_cercano = entrenador_actual;
			distancia_minima = distancia(entrenador_actual->posicion, appeared_pokemon->posicion);
		}
	}

	//modificar_estimacion_y_rafaga(mas_cercano, mas_cercano->rafaga);

	cambiar_estado(mas_cercano, READY);
	cambiar_condicion_ready(mas_cercano);
	t_planificado* planificado = planificado_create(mas_cercano, appeared_pokemon);
	enreadyar(planificado);
	list_add(lista_ready, planificado);

	sem_wait(&mutex_largo_lista_ready);
	largo_lista_ready = list_size(lista_ready);
	sem_post(&mutex_largo_lista_ready);
}

void intercambiar_pokemon_FIFO(t_entrenador* entrenador, t_planificado* planificado) {

	t_entrenador* donador = planificado->entrenador;
	t_appeared_pokemon* auxiliar = planificado->pokemon;

	sem_wait(&(puede_ejecutar[donador->indice]));

	int distance = distancia(entrenador->posicion, donador->posicion);
	int ciclos = distance + INTERCAMBIAR;
	donador->rafaga = ciclos;
	donador->ciclos_cpu += ciclos;

	sem_wait(&(mutex_ciclos_cpu_totales));
	ciclos_cpu_totales += ciclos;
	sem_post(&(mutex_ciclos_cpu_totales));

	mover_de_posicion(donador->posicion, entrenador->posicion, config_team);

	log_info(logger_team, "El entrenador %d esta en la posicion (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);
	log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", donador->indice, donador->posicion->x, donador->posicion->y);

	char* inservible = find_first(donador->objetivos_faltantes, entrenador->pokemon_inservibles);

	intercambiar(entrenador, auxiliar->pokemon, inservible);
	intercambiar(donador, inservible, auxiliar->pokemon);

	for (int i = 0; i < INTERCAMBIAR; i++) {
		sleep(config_team->retardo_ciclo_cpu);
		donador->rafaga--;
	}

	log_info(logger_team, "Entrenador %d recibio a %s y entrego a %s", entrenador->indice, auxiliar->pokemon, inservible);
	log_info(logger_team, "Entrenador %d recibio a %s y entrego a %s", donador->indice, inservible, auxiliar->pokemon);

	// Hay un error
	// No saca los inservibles bien del entrenador
	// pero si lo hace del donador
	// F <- Hay que arreglarlo

	/*printf("entrenador: %d\n", entrenador->indice);

	 for(int i = 0; i < list_size(entrenador->pokemon_obtenidos); i++){
	 char* pokemon = list_get(entrenador->pokemon_obtenidos,i);
	 printf("obtenido: %s\n", pokemon);
	 }

	 for(int i = 0; i < list_size(entrenador->objetivos_faltantes); i++){
	 char* pokemon = list_get(entrenador->objetivos_faltantes,i);
	 printf("objetivo faltante: %s\n", pokemon);
	 }

	 for(int i = 0; i < list_size(entrenador->pokemon_inservibles); i++){
	 char* pokemon = list_get(entrenador->pokemon_inservibles,i);
	 printf("pokemon inservibles: %s\n", pokemon);
	 }

	 printf("donador: %d\n", donador->indice);

	 for(int i = 0; i < list_size(donador->pokemon_obtenidos); i++){
	 char* pokemon = list_get(donador->pokemon_obtenidos,i);
	 printf("obtenido: %s\n", pokemon);
	 }

	 for(int i = 0; i < list_size(donador->objetivos_faltantes); i++){
	 char* pokemon = list_get(donador->objetivos_faltantes,i);
	 printf("objetivo faltante: %s\n", pokemon);
	 }

	 for(int i = 0; i < list_size(donador->pokemon_inservibles); i++){
	 char* pokemon = list_get(donador->pokemon_inservibles,i);
	 printf("pokemon inservibles: %s\n", pokemon);
	 }*/

	free(auxiliar);

	sem_post(&puede_intercambiar);

}

void intercambiar_pokemon_RR(t_entrenador* entrenador, t_planificado* planificado) {

	t_entrenador* donador = planificado->entrenador;
	t_appeared_pokemon* auxiliar = planificado->pokemon;
	sem_wait(&(puede_ejecutar[donador->indice]));

	int distance = distancia(entrenador->posicion, donador->posicion);
	int ciclos = distance + INTERCAMBIAR;
	donador->rafaga = ciclos;
	donador->ciclos_cpu += ciclos;
	int quantum = config_team->quantum;
	int quantum_acumulado = 0;

	sem_wait(&(mutex_ciclos_cpu_totales));
	printf("Ciclos de ejecutar: %d-------------\n", ciclos_cpu_totales);
	ciclos_cpu_totales += ciclos;
	printf("Ciclos de ejecutar: %d-------------\n", ciclos_cpu_totales);
	sem_post(&(mutex_ciclos_cpu_totales));
	for (int i = 0; i < distance; i++) {
		u_int32_t distancia_x = distancia_en_x(entrenador->posicion, donador->posicion);
		for (int i = 0; i < distancia_x; i++) {
			if (esta_mas_a_la_derecha(entrenador->posicion, donador->posicion)) mover_a_la_derecha(donador->posicion);
			else mover_a_la_izquierda(donador->posicion);

			sleep(config_team->retardo_ciclo_cpu);
			quantum_acumulado++;
			donador->rafaga--;

			if (quantum_acumulado == quantum) {
				log_info(logger_team, "El entrenador %d vuelve al fin de la cola de ready por fin de quantum", donador->indice);
				quantum_acumulado = 0;
			}
		}
		u_int32_t distancia_y = distancia_en_y(entrenador->posicion, donador->posicion);
		for (int j = 0; j < distancia_y; j++) {
			if (esta_mas_arriba(entrenador->posicion, donador->posicion)) mover_hacia_arriba(donador->posicion);
			else mover_hacia_abajo(donador->posicion);

			sleep(config_team->retardo_ciclo_cpu);
			quantum_acumulado++;
			donador->rafaga--;

			if (quantum_acumulado == quantum) {
				log_info(logger_team, "El entrenador %d vuelve al fin de la cola de ready por fin de quantum", donador->indice);
				quantum_acumulado = 0;
			}
		}

	}

	log_info(logger_team, "El entrenador %d esta en la posicion (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);
	log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", donador->indice, donador->posicion->x, donador->posicion->y);

	char* inservible = find_first(donador->objetivos_faltantes, entrenador->pokemon_inservibles);

	for (int i = 0; i < INTERCAMBIAR; i++) {
		if (quantum_acumulado == quantum) {
			cambiar_estado(donador, READY);
			queue_push(cola_ready, planificado);
			sem_post(&puede_planificar);
			sem_wait(&puede_ejecutar[donador->indice]);
			quantum_acumulado = 0;
		}
		sleep(config_team->retardo_ciclo_cpu);
		donador->rafaga--;
	}

	intercambiar(entrenador, auxiliar->pokemon, inservible);

	intercambiar(donador, inservible, auxiliar->pokemon);

	log_info(logger_team, "Entrenador %d recibio a %s y entrego a %s", entrenador->indice, auxiliar->pokemon, inservible);
	log_info(logger_team, "Entrenador %d recibio a %s y entrego a %s", donador->indice, inservible, auxiliar->pokemon);

	free(auxiliar);

	sem_post(&puede_intercambiar);
}

void intercambiar_pokemon_SJF(t_entrenador* entrenador, t_planificado* planificado) {

	t_entrenador* donador = planificado->entrenador;
	t_appeared_pokemon* auxiliar = planificado->pokemon;
	sem_wait(&(puede_ejecutar[donador->indice]));

	u_int32_t rafaga = distancia(donador->posicion, entrenador->posicion);
	u_int32_t ciclos = rafaga + INTERCAMBIAR;
	donador->rafaga = ciclos;
	donador->ciclos_cpu += ciclos;

	sem_wait(&(mutex_ciclos_cpu_totales));
	ciclos_cpu_totales += ciclos;
	sem_post(&(mutex_ciclos_cpu_totales));

	mover_de_posicion(donador->posicion, entrenador->posicion, config_team);

	log_info(logger_team, "El entrenador %d esta en la posicion (%d,%d)", entrenador->indice, entrenador->posicion->x, entrenador->posicion->y);
	log_info(logger_team, "El entrenador %d se movió a la posición (%d,%d)", donador->indice, donador->posicion->x, donador->posicion->y);

	char* inservible = find_first(donador->objetivos_faltantes, entrenador->pokemon_inservibles);

	// Pensar en las rafagas para el intercambio
	// Deberia modificar las rafagas de ambos entrenadores (entrenador y donador)?
	// Una solucion es hacerlo solo para el donador.
	// Ya que se mueve e intercambia.
	// Seria la rafaga (cantidad de movimientos) + 5 (intercambio)

	// Falta poner sleep tanto aca como en fifo

	intercambiar(entrenador, auxiliar->pokemon, inservible);
	intercambiar(donador, inservible, auxiliar->pokemon);
	modificar_estimacion_y_rafaga(donador, ciclos);

	for (int i = 0; i < INTERCAMBIAR; i++) {
		sleep(config_team->retardo_ciclo_cpu);
	}

	log_info(logger_team, "Entrenador %d recibio a %s y entrego a %s", entrenador->indice, auxiliar->pokemon, inservible);
	log_info(logger_team, "Entrenador %d recibio a %s y entrego a %s", donador->indice, inservible, auxiliar->pokemon);

	// Hay un error
	// El error que estaba en fifo persiste en sjf
	// seguramente si lo arreglamos en fifo
	// aca sale rapido

	free(auxiliar);

	sem_post(&puede_intercambiar);

}

t_planificado* buscar_donador(t_entrenador* entrenador) {
	t_planificado* planificado = NULL;
	bool encontro_donador = false;
	printf("cantidad de entrenadores en deadlock: %d\n", list_size(entrenadores_deadlock));
	for (int i = 0; i < list_size(entrenadores_deadlock) && !encontro_donador; i++) {
		t_entrenador* donador = list_get(entrenadores_deadlock, i);

		printf("cantidad de inservibles del entrenador %d: %d\n", donador->indice, list_size(donador->pokemon_inservibles));
		for (int j = 0; j < list_size(donador->pokemon_inservibles) && !encontro_donador; j++) {

			char* obtenido = list_get(donador->pokemon_inservibles, j);
			encontro_donador = list_elem(obtenido, entrenador->objetivos_faltantes);
			if (encontro_donador) {

				t_appeared_pokemon* pokemon = appeared_pokemon_create();
				cambiar_nombre_pokemon(pokemon, obtenido);
				cambiar_estado(donador, READY);
				cambiar_condicion_ready(donador);
				planificado = planificado_create(donador, pokemon);
				queue_push(cola_ready, planificado);
				sem_post(&puede_planificar);
			}
		}
	}

	return planificado;
}

void quitar_de_deadlock_fake(uint32_t indice_entrenador, t_list* entrenadores_deadlock_fake){
	bool se_encontro = false;
	t_entrenador* entrenador;
	for(int i=0; i<list_size(entrenadores_deadlock_fake) && !se_encontro; i++){
		entrenador = list_get(entrenadores_deadlock_fake, i);
		se_encontro = (indice_entrenador == (entrenador->indice));
		if(se_encontro){
			list_remove(entrenadores_deadlock_fake, i);
		}
	}
}

void quitar_de_objetivos_faltantes(t_planificado* planificado) {
	bool se_encontro = false;
	char* pokemon;
	for (int i = 0; i < list_size(planificado->entrenador->objetivos_faltantes) && !se_encontro; i++) {
		pokemon = list_get(planificado->entrenador->objetivos_faltantes, i);
		se_encontro = (string_equals_ignore_case(pokemon, planificado->pokemon->pokemon));
		if (se_encontro) {
			list_remove(planificado->entrenador->objetivos_faltantes, i);
		}
	}
}

t_planificado* buscar_donador_simulacion(t_planificado* planificado, t_list* entrenadores_deadlock_fake, t_list* participantes_de_deadlock) {

	t_planificado* planificado_donador = NULL;
	t_entrenador* donador;
	bool encontro_donador = false;
	for (int i = 0; i < list_size(entrenadores_deadlock_fake) && !encontro_donador; i++) {
		donador = list_get(entrenadores_deadlock_fake, i);

		for (int j = 0; j < list_size(donador->pokemon_inservibles) && !encontro_donador; j++) {

			char* obtenido = list_get(donador->pokemon_inservibles, j);

			encontro_donador = string_equals_ignore_case(obtenido, planificado->pokemon->pokemon);

			if(encontro_donador){
				char* pokemon_faltante = list_get(donador->objetivos_faltantes, 0);
				t_appeared_pokemon* pokemon = appeared_pokemon_create();
				cambiar_nombre_pokemon(pokemon, pokemon_faltante);
				planificado_donador = planificado_create(donador, pokemon);
				list_remove_and_destroy_element(donador->pokemon_inservibles, j, free);
				quitar_de_objetivos_faltantes(planificado);

				bool es_entrenador_buscado(void* parametro){
					t_entrenador* entrenador_obtenido = parametro;
					return (entrenador_obtenido->indice) == (donador->indice);
				}

				if(!list_any_satisfy(participantes_de_deadlock, es_entrenador_buscado))
					list_add(participantes_de_deadlock, donador);

			}
			if((list_size(donador->pokemon_inservibles) == 0) && (list_size(donador->objetivos_faltantes) == 0)){
				//list_add(entrenadores_fin_deadlock, donador);
				list_remove(entrenadores_deadlock_fake, i);
			}
			if((list_size(planificado->entrenador->pokemon_inservibles) == 0) && (list_size(planificado->entrenador->objetivos_faltantes) == 0)){
				//list_add(entrenadores_fin_deadlock, planificado->entrenador);
				quitar_de_deadlock_fake(planificado->entrenador->indice, entrenadores_deadlock_fake);
			}
		}
	}

	return planificado_donador;
}

void spoiler_alert(){
	t_list* entrenadores_deadlock_fake = list_duplicate(entrenadores_deadlock);
	t_list* participantes_de_deadlock = list_create();
	t_list* objetivos_faltantes = list_create();
	t_list* pokemons_inservibles = list_create();
	t_entrenador* entrenador_auxiliar;
	char* pokemon_auxiliar;

	for(int i=0;i<list_size(entrenadores_deadlock); i++){
		entrenador_auxiliar = list_get(entrenadores_deadlock, i);
		t_list* sublista_obj_faltantes = list_create();
		for(int j=0; j<list_size(entrenador_auxiliar->objetivos_faltantes); j++){
			pokemon_auxiliar = string_duplicate(list_get(entrenador_auxiliar->objetivos_faltantes, j));
			list_add(sublista_obj_faltantes, pokemon_auxiliar);
		}
		list_add(objetivos_faltantes, sublista_obj_faltantes);

		t_list* sublista_pkm_ins = list_create();
		for (int j = 0; j < list_size(entrenador_auxiliar->pokemon_inservibles); j++) {
			pokemon_auxiliar = string_duplicate(list_get(entrenador_auxiliar->pokemon_inservibles, j));
			list_add(sublista_pkm_ins, pokemon_auxiliar);
		}
		list_add(pokemons_inservibles, sublista_pkm_ins);
	}

	if (!list_is_empty(entrenadores_deadlock_fake)) {
		t_entrenador* entrenador;
		t_planificado* planificado_entrenador;
		char* nombre_pokemon_entrenador;
		t_appeared_pokemon* pokemon_entrenador;
		t_planificado* donador;
		t_entrenador* auxiliar;

		while (!list_is_empty(entrenadores_deadlock_fake)) {
			entrenador = list_get(entrenadores_deadlock_fake, 0);
			nombre_pokemon_entrenador = list_get(entrenador->objetivos_faltantes, 0);
			pokemon_entrenador = appeared_pokemon_create();
			cambiar_nombre_pokemon(pokemon_entrenador, nombre_pokemon_entrenador);
			planificado_entrenador = planificado_create(entrenador, pokemon_entrenador);
			entrenador->cantidad_apariciones_deadlock++;
			list_add(participantes_de_deadlock, entrenador);

			donador = buscar_donador_simulacion(planificado_entrenador, entrenadores_deadlock_fake, participantes_de_deadlock);
			donador->entrenador->cantidad_apariciones_deadlock++;

			while (donador->entrenador->indice != planificado_entrenador->entrenador->indice) {
				t_planificado* auxiliar = buscar_donador_simulacion(donador, entrenadores_deadlock_fake, participantes_de_deadlock);
				free(donador->pokemon);
				free(donador);
				donador = auxiliar;
				donador->entrenador->cantidad_apariciones_deadlock++;
			}

			for (int i = 0; i < list_size(participantes_de_deadlock); i++) {
				entrenador = list_get(participantes_de_deadlock, i);
				cantidad_deadlocks += (entrenador->cantidad_apariciones_deadlock - 1);
				list_remove(participantes_de_deadlock, i);
			}
			for (int j = 0; j < list_size(entrenadores_deadlock_fake); j++) {
				auxiliar = list_get(entrenadores_deadlock_fake, j);
				auxiliar->cantidad_apariciones_deadlock = 0;
				list_clean(participantes_de_deadlock);
			}

			free(planificado_entrenador->pokemon);
			free(planificado_entrenador);
			free(donador->pokemon);
			free(donador);
		}
	}

	printf("-----------------------\n");
	printf("CANTIDAD TOTAL DEADLOCKS: %d\n", cantidad_deadlocks);
	printf("-----------------------\n");

	for (int i=0; i < list_size(entrenadores_deadlock); i++) {
		entrenador_auxiliar = list_get(entrenadores_deadlock, i);
		t_list* sublista1 = list_get(objetivos_faltantes, i);
		list_destroy(entrenador_auxiliar->objetivos_faltantes);
		entrenador_auxiliar->objetivos_faltantes = sublista1;
		t_list* sublista2 = list_get(pokemons_inservibles, i);
		list_destroy(entrenador_auxiliar->pokemon_inservibles);
		entrenador_auxiliar->pokemon_inservibles = sublista2;
	}

	for(int i = 0; i < list_size(entrenadores_deadlock); i++){
		t_entrenador* entrenador = list_get(entrenadores_deadlock,i);
		for(int j = 0; j < list_size(entrenador->objetivos_faltantes); j++){
			char* pokemon = list_get(entrenador->objetivos_faltantes, j);
			printf("objetivo faltante del entrenador %d: %s\n", entrenador->indice, pokemon);
		}

		for(int j = 0; j < list_size(entrenador->pokemon_inservibles); j++){
			char* pokemon = list_get(entrenador->pokemon_inservibles, j);
			printf("inservible del entrenador %d: %s\n", entrenador->indice, pokemon);
		}
	}

	list_destroy(entrenadores_deadlock_fake);
	list_destroy(participantes_de_deadlock);
	list_destroy(pokemons_inservibles);
	list_destroy(objetivos_faltantes);
}


void realizar_intercambios_FIFO() {
	// Falta implementar en otro hilo
	// while segun ALE
	// hasta que se cumplio el objetivo posta posta (literal) segun JOSI
	// implementar con el entrenador global en principio
	// Usamos lista entrenadores_deadlock
	// Buscar dos en deadlock y mandar a uno a planificar y el
	// otro seria variable global, segun ALE

	for (int i = 0; i < list_size(entrenadores); i++) {
		sem_wait(&(termino_de_capturar[i]));
	}

	//REVISAR
	spoiler_alert();

	inicio_deadlock = true;
	sem_post(&sem_appeared_pokemon);

	log_info(logger_team, "Iniciando el algoritmo de deteccion de Deadlock...");

	for(int i = 0; i < list_size(entrenadores_deadlock); i++){
		t_entrenador* entrenador = list_get(entrenadores_deadlock,i);
		for(int j = 0; j < list_size(entrenador->objetivos_faltantes); j++){
			char* pokemon = list_get(entrenador->objetivos_faltantes, j);
			printf("objetivo faltante del entrenador %d: %s\n", entrenador->indice, pokemon);
		}

		for(int j = 0; j < list_size(entrenador->pokemon_inservibles); j++){
			char* pokemon = list_get(entrenador->pokemon_inservibles, j);
			printf("inservible del entrenador %d: %s\n", entrenador->indice, pokemon);
		}
	}


	// Ahora vendria lo que dijo ale
	// Agarro a un entrenador

	while (!list_is_empty(entrenadores_deadlock)) {
		sem_wait(&puede_intercambiar);

		t_entrenador* entrenador = list_head(entrenadores_deadlock);

		if (!list_is_empty(entrenadores_deadlock)) {
			t_planificado* planificado = buscar_donador(entrenador);

			intercambiar_pokemon_FIFO(entrenador, planificado);

			if (no_cumplio_su_objetivo(entrenador)) list_add(entrenadores_deadlock, entrenador);

			if (cumplio_su_objetivo(planificado->entrenador)) sacar_de_los_entrenadores_deadlock(planificado->entrenador);

			free(planificado);
		}
		//cantidad_deadlocks++;
	}

	actualizar_objetivo_global();

	fin_deadlock = true;

	log_info(logger_team, "Fin del algoritmo de deteccion de Deadlock...");

}

void realizar_intercambios_RR() {
	// Falta implementar en otro hilo
	// while segun ALE
	// hasta que se cumplio el objetivo posta posta (literal) segun JOSI
	// implementar con el entrenador global en principio
	// Usamos lista entrenadores_deadlock
	// Buscar dos en deadlock y mandar a uno a planificar y el
	// otro seria variable global, segun ALE

	for (int i = 0; i < list_size(entrenadores); i++) {
		sem_wait(&(termino_de_capturar[i]));
	}

	//REVISAR
	spoiler_alert();

	inicio_deadlock = true;
	sem_post(&sem_appeared_pokemon);

	log_info(logger_team, "Iniciando el algoritmo de deteccion de Deadlock...");

	// Ahora vendria lo que dijo ale
	// Agarro a un entrenador

	while (!list_is_empty(entrenadores_deadlock)) {
		sem_wait(&puede_intercambiar);

		t_entrenador* entrenador = list_head(entrenadores_deadlock);

		if (!list_is_empty(entrenadores_deadlock)) {

			t_planificado* planificado = buscar_donador(entrenador);

			intercambiar_pokemon_RR(entrenador, planificado);

			if (no_cumplio_su_objetivo(entrenador)) list_add(entrenadores_deadlock, entrenador);

			if (cumplio_su_objetivo(planificado->entrenador)) sacar_de_los_entrenadores_deadlock(planificado->entrenador);

			free(planificado);
		}

		//cantidad_deadlocks++;
	}

	actualizar_objetivo_global();

	fin_deadlock = true;

	log_info(logger_team, "Fin del algoritmo de deteccion de Deadlock...");

	sem_post(&(sem_entrenadores));

}

/*
 * @NAME: planificar_entrenadores
 * @DESC: pendiente
 */
void realizar_intercambios_SJF() {

	for (int i = 0; i < list_size(entrenadores); i++) {
		sem_wait(&(termino_de_capturar[i]));
	}

	spoiler_alert();

	inicio_deadlock = true;
	sem_post(&sem_appeared_pokemon);

	log_info(logger_team, "Iniciando el algoritmo de deteccion de Deadlock...");

	while (!list_is_empty(entrenadores_deadlock)) {
		sem_wait(&puede_intercambiar);

		// Aca deberia cambiar
		// No deberia seleccionar el primero en deadlock
		// Deberia seleccionar el que cuyo donador tiene la menor estimacion
		t_list* donadores = buscar_donadores_para_cada_entrenador();
		u_int32_t indice = buscar_entrenador_con_donador_con_estimacion_mas_baja(donadores);

		t_entrenador* entrenador = list_remove(entrenadores_deadlock, indice);

		if (!list_is_empty(entrenadores_deadlock)) {

			// En vez de usar cola ready deberia usar lista ready
			// Aca lo cambie para reutilizar donadores y el indice calculado

			t_planificado* planificado = list_remove(donadores, indice);

			t_entrenador* donador = planificado->entrenador;

			cambiar_estado(donador, READY);
			cambiar_condicion_ready(donador);

			list_add(lista_ready, planificado);
			sem_post(&puede_planificar);

			intercambiar_pokemon_SJF(entrenador, planificado);

			if (no_cumplio_su_objetivo(entrenador)) list_add(entrenadores_deadlock, entrenador);
			if (cumplio_su_objetivo(donador)) sacar_de_los_entrenadores_deadlock(donador);

			free(planificado);
		}

		for (int i = 0; i < list_size(donadores) ;i++){
			t_planificado* planificado = list_get(donadores,i);
			free(planificado->pokemon);
			free(planificado);
		}
		list_destroy(donadores);

		//cantidad_deadlocks++;
	}

	actualizar_objetivo_global();

	fin_deadlock = true;

	log_info(logger_team, "Fin del algoritmo de deteccion de Deadlock...");

}

void planificar_entrenadores() {
	//Falta un switch para planificar según cada algoritmo
	if (!queue_is_empty(cola_ready)) {
		sem_wait(&puede_planificar);
		t_planificado* planificado = queue_pop(cola_ready);
		pokemon_a_atrapar = planificado->pokemon;
		cambiar_estado(planificado->entrenador, EXEC);
		cambios_contexto += 2;
		t_entrenador* auxiliar = planificado->entrenador;
		if (!inicio_deadlock && fue_enreadyado(planificado)) free(planificado);
		sem_post(&(puede_ejecutar[auxiliar->indice]));
	}
	//Mover entrenador
}

// En realidad no creo que sea necesario crear una nueva funcion para esto...
// Simplemente voy a cambiar la estructura de la cola de ready por una lista
// De esa manera voy a poder elegir a cualquier entrenador en ready y no solo
// al primero
// Otro detalle: Cuando no hay nadie en ready, se enreadya siempre
// 				 al mas cercano al pokemon a atrapar y se ejecuta ese.
//               No se tiene en cuenta la estimacion (actua como FIFO).
//               Para mi tiene sentido pero por si las moscas lo digo jajaja
void planificar_entrenadores_SJF() {
	if (!list_is_empty(lista_ready)) {
		sem_wait(&puede_planificar);
		t_planificado* planificado = elegir_proximo_a_ejecutar_SJF();
		pokemon_a_atrapar = planificado->pokemon;
		cambiar_estado(planificado->entrenador, EXEC);
		cambios_contexto += 2;
		t_entrenador* auxiliar = planificado->entrenador;
		if (!inicio_deadlock && fue_enreadyado(planificado)) free(planificado);
		sem_post(&(puede_ejecutar[auxiliar->indice]));
	}
}

void planificar_entrenadores_SJFCD() {
	if (!list_is_empty(lista_ready)) {
		sem_wait(&puede_planificar);
		t_planificado* planificado = elegir_proximo_a_ejecutar_SJFCD();
		pokemon_a_atrapar = planificado->pokemon;
		cambiar_estado(planificado->entrenador, EXEC);
		cambios_contexto += 2;
		t_entrenador* auxiliar = planificado->entrenador;
		if (!inicio_deadlock && fue_enreadyado(planificado)) free(planificado);
		sem_post(&(puede_ejecutar[auxiliar->indice]));
	}
}

t_planificado* elegir_proximo_a_ejecutar_SJFCD() {
	int j = 0;
	t_planificado* planificado = list_get(lista_ready, j);
	t_entrenador* entrenador = planificado->entrenador;
	float estimacion_a_ejecutar = entrenador->estimacion;

	for (int i = 0; i < list_size(lista_ready); i++) {
		t_planificado* planificado_auxiliar = list_get(lista_ready, i);
		t_entrenador* auxiliar = planificado_auxiliar->entrenador;
		float estimacion_actual = auxiliar->estimacion;
		if (estimacion_a_ejecutar > estimacion_actual) {
			estimacion_a_ejecutar = estimacion_actual;
			j = i;
		}
	}

	planificado = list_remove(lista_ready, j);

	sem_wait(&mutex_largo_lista_ready);
	largo_lista_ready = list_size(lista_ready);
	sem_post(&mutex_largo_lista_ready);

	return planificado;
}

// Codigo de prueba
// Pensar cuando calcular la proxima estimacion
t_planificado* elegir_proximo_a_ejecutar_SJF() {
	int j = 0;
	t_planificado* planificado = list_get(lista_ready, j);
	t_entrenador* entrenador = planificado->entrenador;
	float estimacion_a_ejecutar = entrenador->estimacion;

	for (int i = 0; i < list_size(lista_ready); i++) {
		t_planificado* planificado_auxiliar = list_get(lista_ready, i);
		t_entrenador* auxiliar = planificado_auxiliar->entrenador;
		float estimacion_actual = auxiliar->estimacion;
		if (estimacion_a_ejecutar > estimacion_actual) {
			estimacion_a_ejecutar = estimacion_actual;
			j = i;
		}
	}

	planificado = list_remove(lista_ready, j);

	return planificado;
}

//Buscar un mejor nombre jeje
/*
 * @NAME: modificar_estimacion_y_rafaga
 * @DESC: Dados un entrenador y una rafaga, calcula y modifica
 *        la estimacion de la proxima rafaga del entrenador y
 *        modifica su rafaga anterior.
 */
void modificar_estimacion_y_rafaga(t_entrenador* entrenador, u_int32_t rafaga) {
	float estimacion = calcular_estimado_de_la_proxima_rafaga(entrenador->estimacion, rafaga, config_team->alpha);

	set_estimacion(entrenador, estimacion);
	set_rafaga_anterior(entrenador, rafaga);
}

// La cuestion aca es buscar un donador por entrenador
// Y despues elegir al entrenador con el donador con estimacion mas baja
// Sin embargo habria que ver si en vez de buscar un solo donador hay
// que buscar todos los donadores de un entrenador...
// Donadores pasaria a ser una lista de listas
// Y de cada sublista deberia elegir el donador con estimacion mas baja
// Despues ahi si, sigue igual a como esta implementado
t_list* buscar_donadores_para_cada_entrenador() {
	t_list* donadores = list_create();

	for (int i = 0; i < list_size(entrenadores_deadlock); i++) {
		t_entrenador* entrenador = list_get(entrenadores_deadlock, i);
		t_planificado* planificado = buscar_donador_SJF(entrenador);
		list_add(donadores, planificado);
	}

	return donadores;
}

// Codigo de prueba
// En teoria busca al entrenador con el donador con estimacion mas baja
// Deberia ejecutar siempre primero a los de estimacion mas baja para
// coincidir con sjf sin desalojo
u_int32_t buscar_entrenador_con_donador_con_estimacion_mas_baja(t_list* donadores) {
	int j = 0;

	t_planificado* planificado = list_get(donadores, j);

	t_entrenador* donador = planificado->entrenador;
	float estimacion_a_ejecutar = donador->estimacion;

	for (int i = 0; i < list_size(donadores); i++) {
		t_planificado* planificado_auxiliar = list_get(donadores, i);
		t_entrenador* auxiliar = planificado_auxiliar->entrenador;
		float estimacion_actual = auxiliar->estimacion;
		if (estimacion_a_ejecutar > estimacion_actual) {
			estimacion_a_ejecutar = estimacion_actual;
			j = i;
		}
	}

	//t_entrenador* buscado = list_remove(entrenadores_deadlock, j);

	//list_destroy(donadores);
	// Creo que falta destruir cada planificado

	return j;
}

void sacar_de_los_entrenadores_deadlock(t_entrenador* entrenador) {

	int j = 0;

	for (int i = 0; i < list_size(entrenadores_deadlock); i++) {
		t_entrenador* auxiliar = list_get(entrenadores_deadlock, i);
		if (entrenador->indice == auxiliar->indice) j = i;
	}

	list_remove(entrenadores_deadlock, j);

}

// No funciona igual al de fifo
// Es diferente porque no pone en ready al donador
t_planificado* buscar_donador_SJF(t_entrenador* entrenador) {
	t_planificado* planificado = NULL;
	bool encontro_donador = false;
	for (int i = 0; i < list_size(entrenadores_deadlock) && !encontro_donador; i++) {
		t_entrenador* donador = list_get(entrenadores_deadlock, i);
		for (int j = 0; j < list_size(donador->pokemon_inservibles) && !encontro_donador; j++) {
			char* obtenido = list_get(donador->pokemon_inservibles, j);
			encontro_donador = list_elem(obtenido, entrenador->objetivos_faltantes);
			if (encontro_donador) {
				t_appeared_pokemon* pokemon = appeared_pokemon_create();
				cambiar_nombre_pokemon(pokemon, obtenido);
				planificado = planificado_create(donador, pokemon);
			}
		}
	}

	return planificado;
}

/*
 * @NAME: mantener_servidor
 * @DESC: Inicia y mantiene el servidor abierto para escuchar los mensajes
 * 		  que le manden al proceso Team.
 */
void* mantener_servidor() {

	iniciar_servidor();

	return EXIT_SUCCESS;
}

void* iniciar_intercambiador() {

	algoritmo_planificacion algoritmo = get_algoritmo_planificacion(config_team);

	while (!objetivo_global_cumplido()) {

		switch (algoritmo) {
			case (FIFO):
				realizar_intercambios_FIFO();
				break;
			case (RR):
				realizar_intercambios_RR();
				break;
			case (SJF):
				realizar_intercambios_SJF();
				break;
			case (SJFCD):
				realizar_intercambios_SJF();
				break;
		}
		//realizar_intercambios();
		//realizar_intercambios_SJF();
	}

	return EXIT_SUCCESS;
}

/*
 * @NAME: iniciar_planificador
 * @DESC: pendiente
 */

void* iniciar_planificador() {

	algoritmo_planificacion algoritmo = get_algoritmo_planificacion(config_team);

	while (!fin_deadlock) { // hasta que todos terminen

		switch (algoritmo) {
			case (FIFO):
				planificar_entrenadores();
				break;
			case (RR):
				planificar_entrenadores();
				break;
			case (SJF):
				planificar_entrenadores_SJF();
				break;
			case (SJFCD):
				planificar_entrenadores_SJFCD();
				break;
		}
		//planificar_entrenadores();
		//planificar_entrenadores_SJF();
	}
	return EXIT_SUCCESS;
}
/*
 * @NAME: iniciar_planificador_largo_plazo
 * @DESC: Si aparece un pokemon, se encarga de poner en ready al
 * 		  entrenador mas cercano.
 */
void* iniciar_planificador_largo_plazo(void* parametro) {
	t_list* entrenadores = parametro;
	algoritmo_planificacion algoritmo = get_algoritmo_planificacion(config_team);

	while (!pokemons_objetivo_fueron_atrapados()) {
		printf("WARD 1--------------------------------\n");
		sem_wait(&sem_appeared_pokemon);
		printf("WARD 2--------------------------------\n");
		sem_wait(&sem_entrenadores);
		printf("WARD 3--------------------------------\n");
		sem_wait(&sem_planificado_create);
		printf("WARD 4--------------------------------\n");

		if (inicio_deadlock) break;

		printf("WARD 1 EN LA COLA HAY %d POKEMONS-----------------\n", queue_size(appeared_pokemons));

		t_appeared_pokemon* appeared_pokemon = queue_pop(appeared_pokemons);

		printf("WARD 2 EN LA COLA HAY %d POKEMONS-----------------\n", queue_size(appeared_pokemons));

		switch (algoritmo) {
			case FIFO:
				enreadyar_al_mas_cercano(entrenadores, appeared_pokemon);
				break;
			case RR:
				enreadyar_al_mas_cercano(entrenadores, appeared_pokemon);
				break;
			case SJF:
				enreadyar_al_mas_cercano_SJF(entrenadores, appeared_pokemon);
				break;
			case SJFCD:
				enreadyar_al_mas_cercano_SJFCD(entrenadores, appeared_pokemon);
				break;
		}

		//enreadyar_al_mas_cercano(entrenadores, appeared_pokemon);
		//enreadyar_al_mas_cercano_SJF(entrenadores, appeared_pokemon);
	}

	return EXIT_SUCCESS;
}

/*
 * @NAME: elem_especies
 * @DESC: Dados una lista de especies y el nombre de una especie,
 * 		  se fija si ese nombre esta entre las especies.
 */
bool elem_especies(t_list* especies, char* pokemon) {
	bool encontrado = false;
	for (int i = 0; i < list_size(especies) && !encontrado; i++) {
		t_especie* especie = list_get(especies, i);
		encontrado = string_equals_ignore_case(especie->nombre, pokemon);
		if (encontrado) especie->cantidad++;
	}
	return encontrado;
}

/*
 * @NAME: obtener_especies
 * @DESC: Dada una lista con el objetivo global,
 * 		  crea una lista con referencias a estructuras t_especies.
 * 		  Estas especies tienen el nombre de la especie y la cantidad
 * 		  de apariciones en el objetivo global.
 */
t_list* obtener_especies() {

	t_list* especies = list_create();

	for (int i = 0; i < list_size(objetivo_global); i++) {
		char* pokemon = list_get(objetivo_global, i);
		if (elem_especies(especies, pokemon)) {
		}
		else {
			t_especie* especie = malloc(sizeof(t_especie));
			especie->nombre = pokemon;
			especie->cantidad = 1;
			especie->fueRecibida = false;
			list_add(especies, especie);
		}

	}

	return especies;
}

/**
 * @NAME: get_objetivo_global
 * @DESC: Crea y devuelve un puntero a una t_list con los objetivos que faltan
 * 		 atrapar por parte de los entrenadores.
 */
t_list* get_objetivo_global(t_list* entrenadores) {
	return list_map(entrenadores, (void*) get_objetivos_faltantes);
}

/**
 * @NAME: destruir_config_team
 * @DESC: Destruye la estructura t_config_team pasada por parametro.
 */
void destruir_config_team(t_config_team* config_team) {

	list_destroy(config_team->objetivos_entrenadores);
	list_destroy(config_team->pokemon_entrenadores);
	list_destroy(config_team->posiciones_entrenadores);

	free(config_team);
}

/**
 * @NAME: destruir_appeared_pokemons
 * @DESC: Destruye la lista de t_appeared_pokemon.
 */
void destruir_appeared_pokemons() {
	while (!(queue_is_empty(appeared_pokemons))) {
		t_appeared_pokemon* appeared_pokemon = queue_pop(appeared_pokemons);
		appeared_pokemon_destroy(appeared_pokemon);
	}

	queue_destroy(appeared_pokemons);
}

/*
 * @NAME: liberar_estructuras
 * @DESC: Dados un conjunto de estructuras pasados por parametro, se encarga
 * 		 de destruirlas y liberar toda la memoria tomada por ellas.
 * @PARAM: config_team, entrenadores, cola_ready, objetivo_global y
 * 	      especies_requeridas.
 */
void liberar_estructuras(t_config_team* config_team, t_list* entrenadores, t_queue* cola_ready, t_list* objetivo_global, t_list* especies_requeridas) {

	for (int i = 0; i < list_size(entrenadores); i++) {
		t_entrenador* entrenador = list_get(entrenadores, i);
		entrenador_destroy(entrenador);
	}

	destruir_config_team(config_team);
	destruir_appeared_pokemons();

	list_destroy(entrenadores);
	list_destroy(entrenadores_deadlock);
	queue_destroy(cola_ready);
	list_destroy(lista_ready);
	list_destroy(objetivo_global);

	list_iterate(especies_requeridas, free);
	list_destroy(especies_requeridas);

	destruir_vectores_de_semaforos();

}

/*
 * @NAME: terminar_programa
 * @DESC: Destruye las estructuras t_log y t_config pasadas por parametro.
 */
void terminar_programa(t_log* logger, t_config* config) {
	if (logger != NULL) log_destroy(logger);
	if (config != NULL) config_destroy(config);
}

// Codigo de prueba
void liberar_todo(int n) {
	//liberar_estructuras(config_team, entrenadores, cola_ready, objetivo_global, especies_requeridas);
	//terminar_programa(logger_team, config);

	pthread_cancel(hilo_servidor);
	pthread_cancel(hilo_appeared);
	pthread_cancel(hilo_caught);
	pthread_cancel(hilo_localized);
	pthread_cancel(hilo_planificador);
	pthread_cancel(hilo_intercambiador);
	pthread_cancel(hilo_planificador_largo_plazo);

	exit(1);
}

uint32_t obtener_id_segun_cola(char* cola){
	uint32_t id = 0;
	if(string_equals_ignore_case(cola, "APPEARED_POKEMON")) id=id_cola_appeared;
	if(string_equals_ignore_case(cola, "CAUGHT_POKEMON")) id=id_cola_caught;
	if(string_equals_ignore_case(cola, "LOCALIZED_POKEMON")) id=id_cola_localized;

	return id;
}

/*
 * @NAME: suscribirse
 * @DESC: Dado el nombre de una cola, se suscribe a esa cola de mensajes del
 * 		  broker.
 */
void* suscribirse(void* cola) {
	char* msg = (char *) cola;

	int conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);
	printf("Conexion: %d\n", conexion);

	if (conexion < 0){
		return EXIT_SUCCESS;
	}

	char** mensaje = malloc(sizeof(char*) * 4);

	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "SUSCRIPTOR");

	mensaje[2] = string_new();
	string_append(&(mensaje[2]), msg);

	//uint32_t id = obtener_id_segun_cola(msg);

	mensaje[3] = string_itoa(id_team);

	enviar_mensaje(mensaje, conexion);

	for (int i = 0; i < 4; i++)
		free(mensaje[i]);
	free(mensaje);

	pthread_t thread_suscriptor;
	while (!fin_deadlock) { // Falta condicion
		pthread_create(&thread_suscriptor, NULL, (void*) recibir_mensaje, &conexion);
		pthread_join(thread_suscriptor, NULL);
	}

	liberar_conexion(conexion);

	return EXIT_SUCCESS;
}

/*
 * @NAME: suscribirse_a_colas
 * @DESC: Suscribe al proceso Team a las colas de mensajes del
 * 		  broker.
 */
void suscribirse_a_colas() {

	/*char* mensaje = string_new();
	 string_append(&mensaje, "APPEARED_POKEMON");
	 pthread_create(&hilo_appeared, NULL, suscribirse, (void*) mensaje);*/

	pthread_create(&hilo_appeared, NULL, suscribirse, "APPEARED_POKEMON");

	/*mensaje = string_new();
	 string_append(&mensaje, "LOCALIZED_POKEMON");
	 pthread_create(&hilo_localized, NULL, suscribirse, (void*) mensaje);*/

	pthread_create(&hilo_localized, NULL, suscribirse, "LOCALIZED_POKEMON");

	/*mensaje = string_new();
	 string_append(&mensaje, "CAUGHT_POKEMON");
	 pthread_create(&hilo_caught, NULL, suscribirse, (void*) mensaje);*/

	pthread_create(&hilo_caught, NULL, suscribirse, "CAUGHT_POKEMON");

}

/*
 * @NAME: inicializar_vector_de_semaforos
 * @DESC: Dada una longitud devuelve un vector de semáforos inicializados en 0
 *
 */
sem_t* inicializar_vector_de_semaforos(u_int32_t longitud) {

	sem_t* vector = malloc(sizeof(sem_t) * longitud);
	for (int i = 0; i < longitud; i++) {
		sem_init(&(vector[i]), 0, 0);
	}
	return vector;
}

void destruir_vectores_de_semaforos(){
	free(puede_ejecutar);
	free(llega_mensaje_caught);
	free(termino_de_capturar);
}

void actualizar_objetivo_global() {
	t_list* auxiliar = get_objetivo_global(entrenadores);
	if(objetivo_global != NULL) list_destroy(objetivo_global);
	objetivo_global = list_flatten(auxiliar);
	//for(int i=0;i<list_size(auxiliar);i++) list_destroy(list_get(auxiliar,i));
	list_destroy(auxiliar);
	printf("WARD OBJETIVO GLOBAL 2 --------------------\n");
}

bool objetivo_global_cumplido() {
	return list_is_empty(objetivo_global);
}

void informar_resultados() {
	log_info(logger_team, "La cantidad total de ciclos de cpu utilizados fue: %d \n", ciclos_cpu_totales);
	log_info(logger_team, "La cantidad de cambios de contexto realizados fue: %d \n", cambios_contexto);

	for (int i = 0; i < list_size(entrenadores); i++) {
		t_entrenador* entrenador = list_get(entrenadores, i);
		log_info(logger_team, "La cantidad de ciclos de cpu utilizados por el entrenador %d fueron: %d \n", entrenador->indice, entrenador->ciclos_cpu);
	}

	log_info(logger_team, "La cantidad de deadlocks fueron: %d\n", cantidad_deadlocks);
}

void* iniciar_hilo_verificador_de_conexion(){
	int conexion;
	while(1){
		conexion = crear_conexion(config_team->ip_broker, config_team->puerto_broker);
		if(conexion < 0){
			log_info(logger_team, "No se pudo establecer la conexion con el Broker");
			log_info(logger_team, "Se inicia el proceso de reintento de comunicacion con el Broker");
			pthread_cancel(hilo_appeared);
			pthread_cancel(hilo_caught);
			pthread_cancel(hilo_localized);
			sleep(config_team->tiempo_reconexion);
			suscribirse_a_colas();
		}else{
			liberar_conexion(conexion);
			sleep(config_team->tiempo_reconexion);
		}


	}

	return EXIT_SUCCESS;
}

// Funcion main
int main(void) {

	signal(SIGTERM, liberar_todo); // Mostrar
	signal(SIGINT, liberar_todo); // Mostrar
	objetivo_global = NULL;
	config = leer_config();
	ciclos_cpu_totales = 0;
	cambios_contexto = 0;
	cantidad_deadlocks = 0;
	inicio_deadlock = false;
	fin_deadlock = false;
	objetivo_global = NULL;

	config_team = construir_config_team(config);
	logger_team = iniciar_logger(config_team->log_file);

	cola_ready = queue_create();
	lista_ready = list_create(); // Lista para SJF sin desalojo
	largo_lista_ready = 0;
	sem_init(&mutex_largo_lista_ready, 0, 1);

	sem_init(&puede_ser_pusheado, 0, 1);

	appeared_pokemons = queue_create();
	entrenadores_deadlock = list_create();
	id_team = 0;

	sem_init(&sem_appeared_pokemon, 0, 0);
	sem_init(&puede_planificar, 0, 1);
	sem_init(&puede_intercambiar, 0, 1);
	sem_init(&mutex_ciclos_cpu_totales, 0, 1);
	sem_init(&sem_planificado_create, 0, 1);
	puede_ejecutar = inicializar_vector_de_semaforos(list_size(config_team->posiciones_entrenadores));
	llega_mensaje_caught = inicializar_vector_de_semaforos(list_size(config_team->posiciones_entrenadores));

	termino_de_capturar = inicializar_vector_de_semaforos(list_size(config_team->posiciones_entrenadores));

	entrenadores = crear_entrenadores(config_team);

	sem_init(&sem_entrenadores, 0, list_size(entrenadores));

	suscribirse_a_colas();
	actualizar_objetivo_global();

	especies_requeridas = obtener_especies(objetivo_global);

	enviar_mensajes_get_pokemon();
	//pthread_create(&hilo_verificador_de_conexion, NULL, iniciar_hilo_verificador_de_conexion, NULL);
	pthread_create(&hilo_servidor, NULL, mantener_servidor, NULL);
	pthread_create(&hilo_planificador_largo_plazo, NULL, iniciar_planificador_largo_plazo, (void*) entrenadores);
	pthread_create(&hilo_planificador, NULL, iniciar_planificador, NULL);
	pthread_create(&hilo_intercambiador, NULL, iniciar_intercambiador, NULL);

	pthread_join(hilo_intercambiador, NULL);

	pthread_join(hilo_planificador_largo_plazo, NULL);
	pthread_join(hilo_planificador, NULL);

	informar_resultados();
	log_info(logger_team, "El objetivo global fue cumplido \n");

	/*pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_appeared, NULL);
	pthread_join(hilo_caught, NULL);
	pthread_join(hilo_localized, NULL);*/

	printf("LLEGASTE PAPA TE ESTABAMOS ESPERANDO \n");

	//pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_verificador_de_conexion, NULL);
	pthread_cancel(hilo_servidor);
	pthread_cancel(hilo_appeared);
	pthread_cancel(hilo_caught);
	pthread_cancel(hilo_localized);

	liberar_estructuras(config_team, entrenadores, cola_ready, objetivo_global, especies_requeridas);

	terminar_programa(logger_team, config);

	return 0;
}
