/*
 * interfaz_usuario.c
 *
 *  Created on: 21 sept. 2020
 *      Author: t126401
 */

#include "dialogos_json.h"
#include "user_interface.h"
#include "configuracion_usuario.h"
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
#include "funciones_usuario.h"
#include "esp_timer.h"

#include "lv_factory_reset.h"
#include "lv_init_thermostat.h"
#include "lv_thermostat.h"
#include "code_application.h"
#include "configuracion.h"
#include "lv_rgb_main.h"
#include "lvgl.h"


#define CADENCIA_WIFI 250
#define CADENCIA_BROKER 300
#define CADENCIA_SMARTCONFIG 80
#define CADENCIA_SNTP 1000
#define CADENCIA_ALARMA 1500



static const char *TAG = "INTERFAZ_USUARIO";



char* local_event_2_mnemonic(EVENT_DEVICE event) {


	static char mnemonic[50] = {0};

	switch (event) {

	case EVENT_NONE:
		strcpy(mnemonic, "EVENT_NONE");
		break;
		default:
		break;



	}

	ESP_LOGI(TAG, ""TRAZAR"TRADUCCION NEMONICO %d - %s",INFOTRAZA, event, mnemonic);



	return mnemonic;

}


esp_err_t appuser_set_default_config(DATOS_APLICACION *datosApp) {



    //Escribe aqui el codigo de inicializacion por defecto de la aplicacion.
	// Esta funcion es llamada desde el componente configuracion defaultConfig.
	// Aqui puedes establecer los valores por defecto para tu aplicacion.



	return ESP_OK;
}




esp_err_t appuser_notify_no_config(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_smartconfig", INFOTRAZA);



	return ESP_OK;
}

esp_err_t appuser_notify_application_started(DATOS_APLICACION *datosApp) {

/**
 * Introduce el codigo para notificar que la aplicacion ha comenzado. Si estas en este punto,
 * significa que ya estas conectado a la red wifi y a mqtt y por lo tanto ,puedes notificarlo a la aplicacion.
 */




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

esp_err_t appuser_get_date_sntp(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_get_date_sntp", INFOTRAZA);

	return ESP_OK;
}
esp_err_t appuser_notify_error_sntp(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_error_sntp", INFOTRAZA);
	lv_update_alarm_device(datosApp);

	return ESP_OK;
}

esp_err_t appuser_notify_sntp_ok(DATOS_APLICACION *datosApp) {


	ESP_LOGI(TAG, ""TRAZAR"appuser_sntp_ok", INFOTRAZA);



	return ESP_OK;

}



esp_err_t appuser_notify_connecting_wifi(DATOS_APLICACION *datosApp) {


	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_connecting_wifi", INFOTRAZA);

	switch (datosApp->datosGenerales->estadoApp) {

	case FACTORY:

		break;

	case STARTING:
		break;
	default:
		break;

	}



	return ESP_OK;
}

esp_err_t appuser_notify_wifi_connected_ok(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_wifi_connected_ok, ESTADO: %s", INFOTRAZA, status2mnemonic(get_current_status_application(datosApp)));

	return ESP_OK;
}

esp_err_t appuser_notify_error_wifi_connection(DATOS_APLICACION *datosApp) {



	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_error_wifi_connection", INFOTRAZA);
	return ESP_OK;

}



esp_err_t appuser_notify_connecting_broker_mqtt(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_connecting_broker_mqtt", INFOTRAZA);
	return ESP_OK;
}
esp_err_t appuser_notify_broker_connected_ok(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_broker_connected_ok", INFOTRAZA);
	lv_update_alarm_device(datosApp);

	return ESP_OK;
}
esp_err_t appuser_notify_broker_disconnected(DATOS_APLICACION *datosApp) {
	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_broker_disconnected", INFOTRAZA);
	lv_update_alarm_device(datosApp);


	return ESP_OK;
}





void appuser_end_schedule(DATOS_APLICACION *datosApp) {


    ESP_LOGI(TAG, ""TRAZAR"appuser_end_schedule", INFOTRAZA);


}

esp_err_t appuser_start_schedule(DATOS_APLICACION *datosApp) {




	ESP_LOGI(TAG, ""TRAZAR"appuser_start_schedule", INFOTRAZA);


	return ESP_OK;
}

esp_err_t appuser_notify_error_device(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_error_device", INFOTRAZA);


	return ESP_OK;
}


esp_err_t appuser_notify_device_ok(DATOS_APLICACION *datosApp) {
	
	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_local_alarm", INFOTRAZA);



	return ESP_OK;
}


cJSON* appuser_send_spontaneous_report(DATOS_APLICACION *datosApp, enum SPONTANEOUS_TYPE tipoInforme, cJSON *spontaneous) {


    switch(tipoInforme) {


        default:
            printf("enviarReporte--> Salida no prevista\n");
            break;
    }

    return spontaneous;

}
esp_err_t appuser_load_schedule_extra_data(DATOS_APLICACION *datosApp, TIME_PROGRAM *programa_actual, cJSON *nodo) {

	cJSON *item;
	char* dato;


	ESP_LOGI(TAG, ""TRAZAR"appuser_load_schedule_extra_data", INFOTRAZA);

	return ESP_OK;

}

esp_err_t appuser_set_configuration_to_json(DATOS_APLICACION *datosApp, cJSON *conf) {


	ESP_LOGI(TAG, ""TRAZAR"appuser_set_configuration_to_json", INFOTRAZA);

	return ESP_OK;
}

esp_err_t appuser_json_to_configuration(DATOS_APLICACION *datosApp, cJSON *datos) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_json_to_configuration", INFOTRAZA);



	return ESP_OK;
}

esp_err_t appuser_load_default_schedules(DATOS_APLICACION *datosApp, cJSON *array) {




	return ESP_OK;
}

esp_err_t appuser_get_schedule_extra_data(TIME_PROGRAM *programa_actual, cJSON *nodo) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_get_schedule_extra_data", INFOTRAZA);




	return ESP_OK;
}

esp_err_t appuser_modify_schedule_extra_data(TIME_PROGRAM *programa_actual,cJSON *nodo) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_modify_schedule_extra_data", INFOTRAZA);


	return ESP_OK;
}

esp_err_t appuser_reporting_schedule_extra_data(TIME_PROGRAM *programa_actual, cJSON *nodo) {


	ESP_LOGI(TAG, ""TRAZAR"appuser_reporting_schedule_extra_data", INFOTRAZA);

	return ESP_OK;
}




esp_err_t appuser_modify_local_configuration_application(cJSON *root, DATOS_APLICACION *datosApp, cJSON *respuesta) {


	   return salvar_configuracion_general(datosApp);

}


esp_err_t appuser_received_application_device_message(DATOS_APLICACION *datosApp, char *message) {





	return ESP_OK;
}



esp_err_t appuser_notify_app_status(DATOS_APLICACION *datosApp, enum ESTADO_APP estado) {

	char status[50] = {0};

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_app_status", INFOTRAZA);

	switch(datosApp->datosGenerales->estadoApp) {

	case NORMAL_AUTO:
		strcpy(status, "AUTO");
		break;
	case NORMAL_AUTOMAN:
		strcpy(status, "AUTO*");
		break;
	case NORMAL_MANUAL:
		strcpy(status, "MANUAL");
		break;
	case STARTING:
		strcpy(status, "STARTING");
		break;
	case NO_PROGRAM:
		strcpy(status, "NO ACTIVO");
		break;
	case UPGRADING:
		strcpy(status, "UPGRADE EN PROGRESO");
		break;
	case SYNCRONIZING:
		strcpy(status, "SINCRONIZANDO");
		break;
	case WAITING_END_STARTING:
		strcpy(status, "----");
		break;
	case FACTORY:
		strcpy(status, "FACTORY");
		break;
	case NORMAL_FIN_PROGRAMA_ACTIVO:
		strcpy(status, "AUTO");
		break;
	case ERROR_APP:
		strcpy(status, "ERROR_APP");
		break;
	case DEVICE_ALONE:
		strcpy(status, "TERMOSTATO");
		break;
	case CHECK_PROGRAMS:
		strcpy(status, "CHECK");
		break;
	case SCHEDULING:
		strcpy(status, "SCHEDULING");
		break;
	case RESTARTING:
		strcpy(status, "RESTARTING");
		break;


	}


	return ESP_OK;
}


void appuser_notify_schedule_events(DATOS_APLICACION *datosApp) {

	//ESP_LOGI(TAG, ""TRAZAR"appuser_notify_schedule_events", INFOTRAZA);



}





esp_err_t appuser_set_command_application(cJSON *peticion, int nComando, DATOS_APLICACION *datosApp, cJSON *respuesta) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_set_command_application", INFOTRAZA);
    switch(nComando) {


        case STATUS_DEVICE:
            display_status_application(datosApp, respuesta);
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
	lv_create_layout_search_ssid(ap_info, ap_count);


}

void appuser_notify_event_none_schedule(DATOS_APLICACION *datosApp) {


	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_event_none_schedule", INFOTRAZA);


	switch (datosApp->datosGenerales->estadoApp) {

	case NO_PROGRAM:
	case NORMAL_AUTO:
	case NORMAL_AUTOMAN:

		break;

	case CHECK_PROGRAMS:

		break;

	default:
		break;
	}




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




}

void appuser_notify_error_smartconfig(DATOS_APLICACION *datosApp) {

	ESP_LOGE(TAG, ""TRAZAR"Senalizamos el error en smartconfig", INFOTRAZA);


}


