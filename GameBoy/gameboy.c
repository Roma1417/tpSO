#include "gameboy.h"

int main(int argc, char* argv[]){
	int conexion;
	char* ip;
	char* puerto;

	//validar_argumentos(argv, argc-3);
	argv = caso_suscriptor(argv);

	t_log* logger;
	t_config* config;

	logger = iniciar_logger();

	config = leer_config();

	obtener_parametro(&ip, "IP_", argv[1], config);
	obtener_parametro(&puerto, "PUERTO_", argv[1], config);

	conexion = crear_conexion(ip,puerto);
	log_info(logger, "Se realiza una conexión al proceso %s", argv[1]);

	char** p=argv+1;

	enviar_mensaje(p, conexion);
	log_info(logger, "Se ha enviado el mensaje %s al proceso %s", argv[2], argv[1]);

	if(string_equals_ignore_case(argv[2], "SUSCRIPTOR"))
		evaluar_suscripcion(argv, conexion);

	terminar_programa(conexion, logger, config);
}

t_log* iniciar_logger(void){
	t_log* logger = log_create("gameboy.log","gameboy", false, LOG_LEVEL_INFO);
	if(logger == NULL){
		 printf("No pude crear el logger\n");
		 exit(1);
	}
	return logger;
}

t_config* leer_config(void){
	t_config* config = config_create("./gameboy.config");
	if (config == NULL){
		printf("No pude leer la config\n");
		exit(1);
	}
	return config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config){
	if (logger != NULL) log_destroy(logger);
	if (config != NULL) config_destroy(config);
	liberar_conexion(conexion);
}

char* obtener_key(char* parametro, char* destino){
	char *string = string_new();
	string_append(&string, parametro);
	string_append(&string, destino);

	return string;
}

void obtener_parametro(char ** parametro, char* string_parametro, char* destino, t_config* config){
	char* parametro_key = obtener_key(string_parametro, destino);
	*parametro = config_get_string_value(config, parametro_key);
	free(parametro_key);
}

char** caso_suscriptor(char** argv){
	if(string_equals_ignore_case(argv[1], "SUSCRIPTOR")){
		char** parametros = malloc(sizeof(char*)*5);
		parametros[0] = argv[0];
		parametros[1] = "BROKER";
		for(int i=2; i<5; i++){
			parametros[i]=argv[i-1];
		}
		return parametros;
	}
	return argv;
}

void evaluar_suscripcion(char** argv, int conexion){
	int tiempo_a_esperar = atoi(argv[4]);
	//Preguntar por biblioteca para manejar el tiempo del clock
	for(int i=1; i<=tiempo_a_esperar; i++){
		recibir_mensaje(&conexion);
		sleep(1);
		printf("segundos:%d \n", i);
	}
}
