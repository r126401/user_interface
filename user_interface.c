/*
 * interfaz_usuario.c
 *
 *  Created on: 21 sept. 2020
 *      Author: t126401
 */

#include "dialogos_json.h"
#include "user_interface.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "api_json.h"
#include "driver/gpio.h"
#include "conexiones_mqtt.h"
#include "programmer.h"
#include "conexiones.h"
#include "nvslib.h"
#include "events_device.h"
#include "esp_timer.h"
#include "configuracion.h"
#include "code_application.h"
#include "applib.h"





static const char *TAG = "user_interface.c";
#define CADENCIA_STARTING 100 * 1000
#define CADENCIA_WIFI 100 * 1000
#define CADENCIA_BROKER 300 * 1000
#define CADENCIA_SMARTCONFIG 80 * 1000
#define CADENCIA_SNTP 100 * 1000
#define CADENCIA_ALARMA 250 * 1000



esp_err_t appuser_set_default_config(DATOS_APLICACION *datosApp) {




    //Escribe aqui el codigo de inicializacion por defecto de la aplicacion.
	// Esta funcion es llamada desde el componente configuracion defaultConfig.
	// Aqui puedes establecer los valores por defecto para tu aplicacion.



	return ESP_OK;
}




esp_err_t appuser_notify_no_config(DATOS_APLICACION *datosApp) {

	ESP_LOGE(TAG, ""TRAZAR"No hay configuracion", INFOTRAZA);
	activate_timer_led(CADENCIA_SMARTCONFIG);





	return ESP_OK;
}

esp_err_t appuser_notify_application_started(DATOS_APLICACION *datosApp) {

/**
 * Introduce el codigo para notificar que la aplicacion ha comenzado. Si estas en este punto,
 * significa que ya estas conectado a la red wifi y a mqtt y por lo tanto ,puedes notificarlo a la aplicacion.
 */

	ESP_LOGI(TAG, ""TRAZAR"Comienza la aplicacion!!!!!!!!!!!", INFOTRAZA);
	send_spontaneous_report(datosApp, STARTED);





	return ESP_OK;
}

esp_err_t appuser_notify_start_ota(DATOS_APLICACION *datosApp) {


	//Para esp8266 se ha de devolver RESP_RESTART;
	ESP_LOGI(TAG, ""TRAZAR"appuser_start_ota", INFOTRAZA);
	ESP_LOGI(TAG, ""TRAZAR"PUERTO: %d", INFOTRAZA, datosApp->datosGenerales->ota.puerto);
	ESP_LOGI(TAG, ""TRAZAR"servidor ota: %s\n, puerto: %d\n, url: %s, version %s", INFOTRAZA,
			datosApp->datosGenerales->ota.server, datosApp->datosGenerales->ota.puerto, datosApp->datosGenerales->ota.url, datosApp->datosGenerales->ota.swVersion->version);

#ifdef CONFIG_IDF_TARGET_ESP8266
	return RESP_RESTART;
#else

	return ESP_OK;
#endif

}

esp_err_t appuser_notify_get_date_sntp(DATOS_APLICACION *datosApp) {

	ESP_LOGW(TAG, ""TRAZAR"activando ntp", INFOTRAZA);
	cancel_timer_led();
	activate_timer_led(CADENCIA_SNTP);

	return ESP_OK;
}
esp_err_t appuser_notify_error_sntp(DATOS_APLICACION *datosApp) {

	ESP_LOGE(TAG, ""TRAZAR"error al activar ntp", INFOTRAZA);
	cancel_timer_led();
	activate_timer_led(CADENCIA_ALARMA);


	return ESP_OK;
}

esp_err_t appuser_notify_sntp_ok(DATOS_APLICACION *datosApp) {


	ESP_LOGI(TAG, ""TRAZAR"ntp activado", INFOTRAZA);
	cancel_timer_led();



	return ESP_OK;

}



esp_err_t appuser_notify_connecting_wifi(DATOS_APLICACION *datosApp) {


	ESP_LOGW(TAG, ""TRAZAR"Conectandose a la red wifi", INFOTRAZA);
	cancel_timer_led();
	activate_timer_led(CADENCIA_WIFI);



	return ESP_OK;
}

esp_err_t appuser_notify_wifi_connected_ok(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"Wifi conectado, estado: %s", INFOTRAZA, status2mnemonic(get_current_status_application(datosApp)));
	get_my_id();
	cancel_timer_led();

	return ESP_OK;
}

esp_err_t appuser_notify_error_wifi_connection(DATOS_APLICACION *datosApp) {



	ESP_LOGE(TAG, ""TRAZAR"Error al conectarse a la red wifi", INFOTRAZA);
	cancel_timer_led();
	activate_timer_led(CADENCIA_ALARMA);
	return ESP_OK;

}



esp_err_t appuser_notify_connecting_broker_mqtt(DATOS_APLICACION *datosApp) {

	ESP_LOGW(TAG, ""TRAZAR"Conectandose al broker mqtt", INFOTRAZA);
	activate_timer_led(CADENCIA_BROKER);
	return ESP_OK;
}
esp_err_t appuser_notify_broker_connected_ok(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"Conectado al broker mqtt", INFOTRAZA);
	cancel_timer_led();


	return ESP_OK;
}
esp_err_t appuser_notify_broker_disconnected(DATOS_APLICACION *datosApp) {
	ESP_LOGE(TAG, ""TRAZAR"desconectado del broker", INFOTRAZA);
	activate_timer_led(CADENCIA_ALARMA);
	return ESP_OK;
}





void appuser_end_schedule(DATOS_APLICACION *datosApp) {


    ESP_LOGI(TAG, ""TRAZAR"Fin del programa activo", INFOTRAZA);
	relay_operation(datosApp, TEMPORIZADA, OFF);
	send_spontaneous_report(datosApp, END_SCHEDULE);

}

esp_err_t appuser_start_schedule(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"Comienzo del programa activo", INFOTRAZA);
	relay_operation(datosApp, TEMPORIZADA, ON);
	send_spontaneous_report(datosApp, START_SCHEDULE);


	return ESP_OK;
}

esp_err_t appuser_notify_error_device(DATOS_APLICACION *datosApp) {

	ESP_LOGE(TAG, ""TRAZAR"Notificacion de error en el dispositivo", INFOTRAZA);


	return ESP_OK;
}


esp_err_t appuser_notify_device_ok(DATOS_APLICACION *datosApp) {
	
	ESP_LOGI(TAG, ""TRAZAR"El dispositivo esta operativo", INFOTRAZA);



	return ESP_OK;
}


cJSON* appuser_send_spontaneous_report(DATOS_APLICACION *datosApp, enum SPONTANEOUS_TYPE tipoInforme, cJSON *spontaneous) {

	/**
	 * Introduce en esta funcion aquellos elementos que se quieran enviar especificos del dispositivo. Esta informacion se añadira
	 * a la informacion que ya se envia normalmente.
	 */
	ESP_LOGW(TAG, ""TRAZAR"se envia informacion adicional ", INFOTRAZA);
    cJSON_AddNumberToObject(spontaneous, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
    return spontaneous;

}
esp_err_t appuser_load_schedule_extra_data(DATOS_APLICACION *datosApp, TIME_PROGRAM *programa_actual, cJSON *nodo) {


	ESP_LOGW(TAG, ""TRAZAR"Se añade informacion adicional de la aplicacion en el informe", INFOTRAZA);
    if(extraer_dato_int(nodo, DURATION_PROGRAM, (int*) &programa_actual->duracion) != ESP_OK) {
    	programa_actual->duracion = 0;
    }

	return ESP_OK;

}

esp_err_t appuser_set_configuration_to_json(DATOS_APLICACION *datosApp, cJSON *conf) {


	ESP_LOGI(TAG, ""TRAZAR"se vuelca configuracion adicional desde la estructura de datos al json", INFOTRAZA);
	cJSON_AddBoolToObject(conf, WIFI_CONFIG, get_app_config_wifi(datosApp));
	cJSON_AddBoolToObject(conf, MQTT_CONFIG, using_mqtt(datosApp));
	cJSON_AddBoolToObject(conf, TIMING_CONFIG, using_ntp(datosApp));
	cJSON_AddBoolToObject(conf, SCHEDULES_CONFIG, using_schedules(datosApp));


	return ESP_OK;
}

esp_err_t appuser_json_to_configuration(DATOS_APLICACION *datosApp, cJSON *datos) {

	ESP_LOGI(TAG, ""TRAZAR"se vuelca la configuracion adicion json a la estructura de datos.", INFOTRAZA);
	extraer_dato_bool(datos, WIFI_CONFIG, &datosApp->wifi);
	extraer_dato_bool(datos, MQTT_CONFIG, &datosApp->mqtt);
	extraer_dato_bool(datos, TIMING_CONFIG, &datosApp->timing);
	extraer_dato_bool(datos, SCHEDULES_CONFIG, &datosApp->schedules);





	return ESP_OK;
}

esp_err_t appuser_load_default_schedules(DATOS_APLICACION *datosApp, cJSON *array) {




	return ESP_OK;
}

esp_err_t appuser_get_schedule_extra_data(TIME_PROGRAM *programa_actual, cJSON *nodo) {

	ESP_LOGI(TAG, ""TRAZAR"Se extrae configuracion de schedule adicional a la estructura", INFOTRAZA);
    if(extraer_dato_int(nodo, DURATION_PROGRAM, (int*) &programa_actual->duracion) != ESP_OK) {
    	programa_actual->duracion = 0;
    }



	return ESP_OK;
}

esp_err_t appuser_modify_schedule_extra_data(TIME_PROGRAM *programa_actual,cJSON *nodo) {

	ESP_LOGI(TAG, ""TRAZAR"Se modifica inforamcion adicional del schedule", INFOTRAZA);
	extraer_dato_uint32(nodo, DURATION_PROGRAM, &programa_actual->duracion);


	return ESP_OK;
}

esp_err_t appuser_reporting_schedule_extra_data(TIME_PROGRAM *programa_actual, cJSON *nodo) {


	ESP_LOGI(TAG, ""TRAZAR"se pinta informacion adicional de la estructura del schedule", INFOTRAZA);
    if (programa_actual->duracion > 0) {
        cJSON_AddNumberToObject(nodo, DURATION_PROGRAM, programa_actual->duracion);

    }


	return ESP_OK;
}




esp_err_t appuser_modify_local_configuration_application(cJSON *root, DATOS_APLICACION *datosApp, cJSON *respuesta) {


	ESP_LOGW(TAG, ""TRAZAR" se modifica configuracion adicional del dispositivo", INFOTRAZA);
	return salvar_configuracion_general(datosApp);

}


esp_err_t appuser_received_application_device_message(DATOS_APLICACION *datosApp, char *message) {

	ESP_LOGW(TAG, ""TRAZAR"recibido mensaje específico para la aplicacion", INFOTRAZA);



	return ESP_OK;
}



esp_err_t appuser_notify_app_status(DATOS_APLICACION *datosApp, enum ESTADO_APP estado) {


	ESP_LOGI(TAG, ""TRAZAR"realizado cambio de estado: estado :%s", INFOTRAZA, status2mnemonic(estado));





	return ESP_OK;
}


void appuser_notify_schedule_events(DATOS_APLICACION *datosApp) {

	/**
	 * Evento que se recibe cada segundo por si se quiere realizar una accion adicional.
	 * Escribe aqui el codigo que quieres que se realice cada segundo.
	 */

	//ESP_LOGW(TAG, ""TRAZAR"notificacion adicional de la gestion de programas", INFOTRAZA);



}





esp_err_t appuser_set_command_application(cJSON *peticion, int nComando, DATOS_APLICACION *datosApp, cJSON *respuesta) {

	ESP_LOGI(TAG, ""TRAZAR"recibido comando de aplicacion", INFOTRAZA);



    switch(nComando) {



        case OPERATION_REMOTE_RELAY:
        	relay_operation(datosApp, REMOTA, INDETERMINADO);
        	appuser_set_relay_operation_command(datosApp, respuesta);
        	break;

        case STATUS_LOCAL_DEVICE:
        	appuser_get_status_command(datosApp, respuesta);
            break;

        default:
            visualizar_comando_desconocido(datosApp, respuesta);
            break;


    }




	return ESP_OK;
}



void appuser_notify_scan_done(DATOS_APLICACION *datosApp, wifi_ap_record_t *ap_info, uint16_t *ap_count) {


	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_scan_done", INFOTRAZA);
	ESP_LOGI(TAG, ""TRAZAR"RECIBIDAS %d redes en app", INFOTRAZA, *ap_count);



}

void appuser_notify_event_no_active_schedule(DATOS_APLICACION *datosApp) {


	ESP_LOGI(TAG, ""TRAZAR"Ningun schedule activo", INFOTRAZA);



}





void appuser_received_local_event(DATOS_APLICACION *datosApp, EVENT_DEVICE event) {



	ESP_LOGI(TAG, ""TRAZAR"appuser_received_local_event: recibido :%s", INFOTRAZA, local_event_2_mnemonic(event));


}




void appuser_notify_alarm_on_device(DATOS_APLICACION *datosApp, ALARM_TYPE alarm) {


	ESP_LOGE(TAG, ""TRAZAR"ALARMA %s en estado ON", INFOTRAZA, datosApp->alarmas[alarm].nemonico);
	switch (alarm) {

	case ALARM_REMOTE_DEVICE:

		break;
	default:
		break;
	}







}


void appuser_notify_alarm_off_device(DATOS_APLICACION *datosApp, ALARM_TYPE alarm) {


	ESP_LOGE(TAG, ""TRAZAR"ALARMA %s en estado OFF", INFOTRAZA, datosApp->alarmas[alarm].nemonico);
	switch (alarm) {

	case ALARM_REMOTE_DEVICE:

		break;
	default:
		break;
	}

}

void appuser_notify_error_remote_device(DATOS_APLICACION *datosApp) {



	ESP_LOGI(TAG, ""TRAZAR"LEEMOS EN LOCAL PORQUE NO RESPONDE EL DISPOSITIVO REMOTO", INFOTRAZA);





}

void appuser_notify_smartconfig_end(DATOS_APLICACION *datosApp) {


	cancel_timer_led();


}

void appuser_notify_error_smartconfig(DATOS_APLICACION *datosApp) {

	ESP_LOGE(TAG, ""TRAZAR"Senalizamos el error en smartconfig", INFOTRAZA);
	activate_timer_led(CADENCIA_ALARMA);


}


