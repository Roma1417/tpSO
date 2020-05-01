#include "gameboy.h"

int main(int argc, char* argv[]){

	printf("argc es igual a: %d\n", argc);
	for(int i=0; i<argc; i++){
		printf("argv[%d] es igual a: %s\n", i, argv[i]);
	}

	int conexion;
	char* ip;
	char* puerto;

	t_log* logger;
	t_config* config;

	logger = iniciar_logger();

	config = leer_config();

	ip = config_get_string_value(config, obtener_key("IP_", argv[1]));
	puerto = config_get_string_value(config, obtener_key("PUERTO_", argv[1]));

	log_info(logger, "El IP es: %s", ip);
	log_info(logger, "El PUERTO es: %s", puerto);

	conexion = crear_conexion(ip,puerto);


	char** p=argv+2;

	enviar_mensaje(p, conexion, argc - 3);

	//char* unMensaje = recibir_mensaje(conexion);

	//log_info(logger, "El mensaje recibido es: %s", unMensaje);

	//free(unMensaje);

	terminar_programa(conexion, logger, config);
}

t_log* iniciar_logger(void){
	t_log* logger = log_create("gameboy.log","gameboy", true, LOG_LEVEL_INFO);
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
