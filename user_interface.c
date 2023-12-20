/*
 * interfaz_usuario.c
 *
 *  Created on: 21 sept. 2020
 *      Author: t126401
 */

#include "dialogos_json.h"
#include "user_interface.h"
//#include "configuracion_usuario.h"
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
//#include "funciones_usuario.h"
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
	case EVENT_UP_THRESHOLD:
		strcpy(mnemonic, "EVENT_UP_THRESHOLD");
		break;
	case EVENT_DOWN_THRESHOLD:
		strcpy(mnemonic, "EVENT_DOWN_THRESHOLD");
		break;
	case EVENT_RELAY_ON:
		strcpy(mnemonic, "EVENT_RELAY_ON");
		break;
	case EVENT_RELAY_OFF:
		strcpy(mnemonic, "EVENT_RELAY_OFF");
		break;
	case EVENT_ANSWER_TEMPERATURE:
		strcpy(mnemonic, "EVENT_ANSWER_TEMPERATURE");
		break;
	case EVENT_REQUEST_TEMPERATURE:
		strcpy(mnemonic, "EVENT_REQUEST_TEMPERATURE");
		break;
	case EVENT_TIMEOUT_REMOTE_TEMPERATURE:
		strcpy(mnemonic, "EVENT_ERROR_REMOTE_TEMPERATURE");
		break;
	case EVENT_ERROR_READ_LOCAL_TEMPERATURE:
		strcpy(mnemonic, "EVENT_ERROR_READ_LOCAL_TEMPERATURE");
		break;
	case EVENT_WAITING_RESPONSE_TEMPERATURE:
		strcpy(mnemonic, "EVENT_WAITING_RESPONSE_TEMPERATURE");
		break;




	}

	ESP_LOGI(TAG, ""TRAZAR"TRADUCCION NEMONICO %d - %s",INFOTRAZA, event, mnemonic);



	return mnemonic;

}


esp_err_t appuser_set_default_config(DATOS_APLICACION *datosApp) {



    //Escribe aqui el codigo de inicializacion por defecto de la aplicacion.
	// Esta funcion es llamada desde el componente configuracion defaultConfig.
	// Aqui puedes establecer los valores por defecto para tu aplicacion.
	ESP_LOGI(TAG, ""TRAZAR"appuser_set_default_config", INFOTRAZA);
	if (datosApp->datosGenerales->tipoDispositivo == DESCONOCIDO) {
		datosApp->datosGenerales->tipoDispositivo = CRONOTERMOSTATO;
	}
    datosApp->termostato.reintentosLectura = 5;
    datosApp->termostato.retry_interval = 3;
    datosApp->termostato.margenTemperatura = 0.5;
    datosApp->termostato.read_interval = 10;
    datosApp->termostato.tempUmbral = 21.5;
    datosApp->termostato.tempUmbralDefecto = 21.5;
    datosApp->termostato.calibrado = -2.0;
    datosApp->termostato.master = true;
    datosApp->termostato.incdec = 0.5;
    memset(datosApp->termostato.sensor_remoto, 0, sizeof(datosApp->termostato.sensor_remoto));


	return ESP_OK;
}




esp_err_t appuser_notify_no_config(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_smartconfig", INFOTRAZA);
	lv_configure_smartconfig();
	lv_factory_boot();
	lv_cancel_timing_backlight();

	//lv_timer_handler();


	return ESP_OK;
}

esp_err_t appuser_notify_application_started(DATOS_APLICACION *datosApp) {

/**
 * Introduce el codigo para notificar que la aplicacion ha comenzado. Si estas en este punto,
 * significa que ya estas conectado a la red wifi y a mqtt y por lo tanto ,puedes notificarlo a la aplicacion.
 */

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_application_started. Estado Aplicacion: %s", INFOTRAZA, status2mnemonic(datosApp->datosGenerales->estadoApp));
    if (datosApp->termostato.master == false) {
    	ESP_LOGI(TAG, ""TRAZAR"sensor remoto. Nos subscribimos a %s", INFOTRAZA, datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].subscribe);
    	if (subscribe_topic(datosApp, datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].subscribe) == ESP_OK) {
			datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].status = true;
    		send_event(__func__,EVENT_REMOTE_DEVICE_OK);
    	} else {
    		send_event(__func__,EVENT_ERROR_REMOTE_DEVICE);
			datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].status = false;
    	}
    }


	lv_update_temperature(datosApp);


	ESP_LOGW(TAG, ""TRAZAR" vamos a publicar el arranque del dispositivo", INFOTRAZA);
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

	char fecha_actual[10] = {0};
	time_t now;
	struct tm fecha;
	ESP_LOGI(TAG, ""TRAZAR"appuser_sntp_ok", INFOTRAZA);
	lv_update_alarm_device(datosApp);

    time(&now);
    localtime_r(&now, &fecha);
    sprintf(fecha_actual, "%02d:%02d", fecha.tm_hour, fecha.tm_min);
    ESP_LOGI(TAG, ""TRAZAR"hora actualizada: %s", INFOTRAZA, fecha_actual);
    lv_update_hour(fecha_actual);


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
	lv_update_alarm_device(datosApp);


	return ESP_OK;
}

esp_err_t appuser_notify_error_wifi_connection(DATOS_APLICACION *datosApp) {



	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_error_wifi_connection", INFOTRAZA);
	lv_update_alarm_device(datosApp);

	lv_configure_smartconfig();
	lv_factory_boot();

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
    datosApp->termostato.tempUmbral = datosApp->termostato.tempUmbralDefecto;
    lv_update_threshold(datosApp, true);
    lv_update_bar_schedule(datosApp, false);
    send_spontaneous_report(datosApp, END_SCHEDULE);
    ESP_LOGI(TAG, ""TRAZAR"FIN DE LA TEMPORIZACION. SE PASA A LA TEMPERATURA DE DEFECTO", INFOTRAZA);

}

esp_err_t appuser_start_schedule(DATOS_APLICACION *datosApp) {


	enum ESTADO_RELE accion;

	ESP_LOGI(TAG, ""TRAZAR"appuser_start_schedule", INFOTRAZA);
	datosApp->termostato.tempUmbral = datosApp->datosGenerales->programacion[datosApp->datosGenerales->nProgramaCandidato].temperatura;

    if (calcular_accion_termostato(datosApp, &accion) == ACCIONAR_TERMOSTATO) {
    	relay_operation(datosApp, TEMPORIZADA, accion);

    }
	send_spontaneous_report(datosApp, START_SCHEDULE);
	lv_update_threshold(datosApp, true);
	// actualizar los intervalos del lcd
	lv_update_bar_schedule(datosApp, true);

	return ESP_OK;
}

esp_err_t appuser_notify_error_device(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_error_device", INFOTRAZA);
	lv_update_alarm_device(datosApp);


	return ESP_OK;
}


esp_err_t appuser_notify_device_ok(DATOS_APLICACION *datosApp) {
	
	ESP_LOGI(TAG, ""TRAZAR"appuser_notify_local_alarm", INFOTRAZA);
	lv_update_alarm_device(datosApp);


	return ESP_OK;
}


cJSON* appuser_send_spontaneous_report(DATOS_APLICACION *datosApp, enum SPONTANEOUS_TYPE tipoInforme, cJSON *spontaneous) {


    switch(tipoInforme) {
        case STARTED:
            cJSON_AddNumberToObject(spontaneous, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
            cJSON_AddNumberToObject(spontaneous, TEMPERATURA, datosApp->termostato.tempActual);
#ifdef CONFIG_DHT22
            cJSON_AddNumberToObject(spontaneous, HUMEDAD, datosApp->termostato.humedad);
#endif
            cJSON_AddNumberToObject(spontaneous, UMBRAL_TEMPERATURA, datosApp->termostato.tempUmbral);
            cJSON_AddBoolToObject(spontaneous, MASTER, datosApp->termostato.master);
            cJSON_AddStringToObject(spontaneous, SENSOR_REMOTO, datosApp->termostato.sensor_remoto);
            break;

        case OP_LOCAL_RELAY:
        case START_SCHEDULE:
        case END_SCHEDULE:
        case CHANGE_TEMPERATURE:
        case CAMBIO_UMBRAL_TEMPERATURA:
            cJSON_AddNumberToObject(spontaneous, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
            cJSON_AddNumberToObject(spontaneous, TEMPERATURA, datosApp->termostato.tempActual);
#ifdef CONFIG_DHT22
            cJSON_AddNumberToObject(spontaneous, HUMEDAD, datosApp->termostato.humedad);
#endif
            cJSON_AddNumberToObject(spontaneous, UMBRAL_TEMPERATURA, datosApp->termostato.tempUmbral);
            cJSON_AddBoolToObject(spontaneous, MASTER, datosApp->termostato.master);
            cJSON_AddStringToObject(spontaneous, SENSOR_REMOTO, datosApp->termostato.sensor_remoto);
             break;
        case START_UPGRADE_OTA:
        	break;

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
	item = cJSON_GetObjectItem(nodo, PROGRAM_ID);
	dato = cJSON_GetStringValue(item);
	if (item != NULL) {
		switch(programa_actual->tipo) {
		case DIARIA:
			programa_actual->accion = extraer_dato_tm(dato, 11, 1);
			break;
		case SEMANAL:
			break;
		case FECHADA:
			programa_actual->accion = extraer_dato_tm(dato, 17, 1);
			break;
		}


	}

	item = cJSON_GetObjectItem(nodo, DURATION_PROGRAM);
	if (item != NULL) {
		programa_actual->duracion = item->valueint;
		ESP_LOGI(TAG, ""TRAZAR"DURACION = "CONFIG_UINT32_FORMAT"", INFOTRAZA, programa_actual->duracion);
	} else {
		programa_actual->duracion = 0;
		ESP_LOGI(TAG, ""TRAZAR"NO SE GUARDA DURACION: "CONFIG_UINT32_FORMAT"", INFOTRAZA, programa_actual->duracion);
	}

	ESP_LOGI(TAG, ""TRAZAR" VAMOS A CALCULAR LA TEMPERATURA UMBRAL", INFOTRAZA);
	extraer_dato_double(nodo, UMBRAL_TEMPERATURA, &programa_actual->temperatura);
	datosApp->termostato.tempUmbral = programa_actual->temperatura;
	ESP_LOGI(TAG, ""TRAZAR" UMBRAL :%lf", INFOTRAZA, programa_actual->temperatura);
	lv_update_threshold(datosApp, true);


	return ESP_OK;

}

esp_err_t appuser_set_configuration_to_json(DATOS_APLICACION *datosApp, cJSON *conf) {

	//cJSON_AddNumberToObject(conf, DEVICE , INTERRUPTOR);
	ESP_LOGI(TAG, ""TRAZAR"appuser_set_configuration_to_json", INFOTRAZA);

    cJSON_AddNumberToObject(conf, MARGEN_TEMPERATURA, datosApp->termostato.margenTemperatura);
    cJSON_AddNumberToObject(conf, INTERVALO_LECTURA, datosApp->termostato.read_interval);
    cJSON_AddNumberToObject(conf, INTERVALO_REINTENTOS, datosApp->termostato.retry_interval);
    cJSON_AddNumberToObject(conf, REINTENTOS_LECTURA, datosApp->termostato.reintentosLectura);
    cJSON_AddNumberToObject(conf, CALIBRADO, datosApp->termostato.calibrado);
    cJSON_AddBoolToObject(conf, MASTER, datosApp->termostato.master);
    cJSON_AddStringToObject(conf, SENSOR_REMOTO, datosApp->termostato.sensor_remoto);
    cJSON_AddNumberToObject(conf, UMBRAL_DEFECTO, datosApp->termostato.tempUmbralDefecto);
    cJSON_AddNumberToObject(conf, INCDEC, datosApp->termostato.incdec);
	return ESP_OK;
}

esp_err_t appuser_json_to_configuration(DATOS_APLICACION *datosApp, cJSON *datos) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_json_to_configuration", INFOTRAZA);
	extraer_dato_double(datos, MARGEN_TEMPERATURA, &datosApp->termostato.margenTemperatura);
	extraer_dato_uint8(datos, INTERVALO_LECTURA, &datosApp->termostato.read_interval);
	extraer_dato_uint8(datos, INTERVALO_REINTENTOS, &datosApp->termostato.retry_interval);
	extraer_dato_uint8(datos, REINTENTOS_LECTURA, &datosApp->termostato.reintentosLectura);
	extraer_dato_double(datos, CALIBRADO, &datosApp->termostato.calibrado);
	extraer_dato_uint8(datos,  MASTER, (uint8_t*) &datosApp->termostato.master);
	extraer_dato_string(datos, SENSOR_REMOTO, datosApp->termostato.sensor_remoto);
	extraer_dato_float(datos,  UMBRAL_DEFECTO, &datosApp->termostato.tempUmbralDefecto);
	extraer_dato_float(datos, INCDEC, &datosApp->termostato.incdec);


	return ESP_OK;
}

esp_err_t appuser_load_default_schedules(DATOS_APLICACION *datosApp, cJSON *array) {

	cJSON *item = NULL;

	ESP_LOGI(TAG, ""TRAZAR"appuser_load_default_schedules", INFOTRAZA);
	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "000000007f11");
	cJSON_AddNumberToObject(item, UMBRAL_TEMPERATURA, 21.5);
	cJSON_AddNumberToObject(item, DURATION_PROGRAM, 3600);
	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "001200007f11");
	cJSON_AddNumberToObject(item, UMBRAL_TEMPERATURA, 22.5);
	cJSON_AddNumberToObject(item, DURATION_PROGRAM, 18000);
	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "001900007f11");
	cJSON_AddNumberToObject(item, UMBRAL_TEMPERATURA, 23.5);
	cJSON_AddNumberToObject(item, DURATION_PROGRAM, 13800);
	cJSON_AddItemToArray(array, item = cJSON_CreateObject());
	cJSON_AddStringToObject(item, PROGRAM_ID, "001038007f11");
	cJSON_AddNumberToObject(item, UMBRAL_TEMPERATURA, 24.5);
	cJSON_AddNumberToObject(item, DURATION_PROGRAM, 3600);


	return ESP_OK;
}

esp_err_t appuser_get_schedule_extra_data(TIME_PROGRAM *programa_actual, cJSON *nodo) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_get_schedule_extra_data", INFOTRAZA);
	extraer_dato_double(nodo, UMBRAL_TEMPERATURA, &programa_actual->temperatura);
	extraer_dato_double(nodo, HUMEDAD, &programa_actual->humedad);
    if(extraer_dato_int(nodo, DURATION_PROGRAM, (int*) &programa_actual->duracion) != ESP_OK) {
    	programa_actual->duracion = 0;
    }



	return ESP_OK;
}

esp_err_t appuser_modify_schedule_extra_data(TIME_PROGRAM *programa_actual,cJSON *nodo) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_modify_schedule_extra_data", INFOTRAZA);
	extraer_dato_double(nodo, UMBRAL_TEMPERATURA, &programa_actual->temperatura);
	extraer_dato_double(nodo, HUMEDAD, &programa_actual->humedad);
	extraer_dato_uint32(nodo, DURATION_PROGRAM, &programa_actual->duracion);


	return ESP_OK;
}

esp_err_t appuser_reporting_schedule_extra_data(TIME_PROGRAM *programa_actual, cJSON *nodo) {


	ESP_LOGI(TAG, ""TRAZAR"appuser_reporting_schedule_extra_data", INFOTRAZA);
    cJSON_AddNumberToObject(nodo, UMBRAL_TEMPERATURA, programa_actual->temperatura);
    if (programa_actual->duracion > 0) {
        cJSON_AddNumberToObject(nodo, DURATION_PROGRAM, programa_actual->duracion);

    }

	return ESP_OK;
}


esp_err_t set_topic_remote_sensor(DATOS_APLICACION *datosApp, char *id_remote_sensor) {

	strcpy(datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].publish, CONFIG_PREFIX_TOPIC_PUBLISH_REMOTE_TEMPERATURE);
	strcpy(datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].subscribe, CONFIG_PREFIX_TOPIC_SUBSCRIBE_REMOTE_TEMPERATURE);

	strcat(datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].publish, id_remote_sensor);
	strcat(datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].subscribe, id_remote_sensor);
	datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].status = true;


	return ESP_OK;

}


esp_err_t appuser_modify_local_configuration_application(cJSON *root, DATOS_APLICACION *datosApp, cJSON *respuesta) {

	   cJSON *nodo;

	   ESP_LOGI(TAG, ""TRAZAR"appuser_modify_local_configuration_application", INFOTRAZA);
	   nodo = cJSON_GetObjectItem(root, APP_PARAMS);
	   if(nodo == NULL) {
	       codigoRespuesta(respuesta, RESP_NOK);
	       return ESP_FAIL;
	   }

	   extraer_dato_uint8(nodo, MQTT_TLS, (uint8_t*) &datosApp->datosGenerales->parametrosMqtt.tls);
	   if (datosApp->datosGenerales->parametrosMqtt.tls == false) {
		   datosApp->datosGenerales->parametrosMqtt.port = 1883;
		   strcpy(datosApp->datosGenerales->parametrosMqtt.broker, (const char*) "mqtt://jajicaiot.ddns.net");
	   } else {
		   datosApp->datosGenerales->parametrosMqtt.port = 8883;
		  	   strcpy(datosApp->datosGenerales->parametrosMqtt.broker, (const char*) "mqtts://jajicaiot.ddns.net");
	   }
	   extraer_dato_double(nodo, MARGEN_TEMPERATURA, &datosApp->termostato.margenTemperatura);
	   extraer_dato_uint8(nodo, INTERVALO_LECTURA, &datosApp->termostato.read_interval);
	   extraer_dato_uint8(nodo, INTERVALO_REINTENTOS, &datosApp->termostato.retry_interval);
	   extraer_dato_uint8(nodo, REINTENTOS_LECTURA, &datosApp->termostato.reintentosLectura);
	   extraer_dato_double(nodo, CALIBRADO, &datosApp->termostato.calibrado);
	   extraer_dato_float(nodo, UMBRAL_DEFECTO, &datosApp->termostato.tempUmbralDefecto);
	   extraer_dato_float(nodo, INCDEC, &datosApp->termostato.incdec);
	   if ((extraer_dato_uint8(nodo,  MASTER, (uint8_t*) &datosApp->termostato.master)) == ESP_OK) {


		   if (datosApp->termostato.master) {
			   ESP_LOGI(TAG, ""TRAZAR" Se modifica el sensor para que el master sea el dispositivo", INFOTRAZA);
			   memset(datosApp->termostato.sensor_remoto, 0,sizeof(datosApp->termostato.sensor_remoto));
			   unsubscribe_topic(datosApp, CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE);
		   } else {
			   ESP_LOGE(TAG, ""TRAZAR"Se configura sensor remoto: "CONFIG_UINT32_FORMAT"\n", INFOTRAZA, esp_get_free_heap_size());
			   extraer_dato_string(nodo, SENSOR_REMOTO, datosApp->termostato.sensor_remoto);
			   ESP_LOGE(TAG, ""TRAZAR"despues: "CONFIG_UINT32_FORMAT"\n", INFOTRAZA, esp_get_free_heap_size());
			   ESP_LOGW(TAG, ""TRAZAR" Se selecciona el sensor remoto a :%s", INFOTRAZA, datosApp->termostato.sensor_remoto);
			   set_topic_remote_sensor(datosApp, datosApp->termostato.sensor_remoto);
			   subscribe_topic(datosApp, datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].subscribe);
			   cJSON_AddStringToObject(respuesta, SENSOR_REMOTO, datosApp->termostato.sensor_remoto);

		   }

		   cJSON_AddBoolToObject(respuesta, MASTER, datosApp->termostato.master);

	   }
	   codigoRespuesta(respuesta, RESP_OK);

	   ESP_LOGI(TAG, ""TRAZAR" parametros de configuracion  modificados", INFOTRAZA);
	   return salvar_configuracion_general(datosApp);

}


esp_err_t appuser_received_application_device_message(DATOS_APLICACION *datosApp, char *message) {



		appuser_reading_remote_temperature(datosApp, message);



	return ESP_OK;
}


esp_err_t appuser_reading_remote_temperature(DATOS_APLICACION *datosApp, char *message) {


	cJSON *respuesta;
	double dato;
	float temperatura_a_redondear;

	ESP_LOGI(TAG, ""TRAZAR"appuser_received_application_device_message", INFOTRAZA);
	respuesta = cJSON_Parse(message);
	if (respuesta != NULL) {
		extraer_dato_double(respuesta, TEMPERATURA, &dato);
		datosApp->termostato.tempActual = (float) dato;
		extraer_dato_double(respuesta, HUMEDAD, &dato);
		temperatura_a_redondear = datosApp->termostato.tempActual;
      	datosApp->termostato.tempActual = redondear_temperatura(temperatura_a_redondear);

		datosApp->termostato.humedad = (float) dato;
		ESP_LOGI(TAG, ""TRAZAR" temperatura remota :%lf, humedad remota:%lf, %s", INFOTRAZA, datosApp->termostato.tempActual,datosApp->termostato.humedad, __func__);
		cJSON_Delete(respuesta);
		send_event_device(__func__,EVENT_ANSWER_TEMPERATURE);
		return ESP_OK;

	} else {
		return ESP_FAIL;
		send_event(__func__,EVENT_ERROR_REMOTE_DEVICE);
	}



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


	lv_update_status_application(status);
	return ESP_OK;
}


void appuser_notify_schedule_events(DATOS_APLICACION *datosApp) {

	//ESP_LOGI(TAG, ""TRAZAR"appuser_notify_schedule_events", INFOTRAZA);

	char fecha_actual[10] = {0};
	time_t now;
	struct tm fecha;
	time_t t_siguiente_intervalo;


    time(&now);
    localtime_r(&now, &fecha);
    if (fecha.tm_sec == 0) {

    	sprintf(fecha_actual, "%02d:%02d", fecha.tm_hour, fecha.tm_min);
    	ESP_LOGI(TAG, ""TRAZAR"hora actualizada: %s", INFOTRAZA, fecha_actual);
    	lv_update_hour(fecha_actual);

    	if (calcular_programa_activo(datosApp, &t_siguiente_intervalo) == ACTIVE_SCHEDULE) {

    		ESP_LOGE(TAG, ""TRAZAR"ACTUALIZAMOS EL BAR SCHEDULE", INFOTRAZA);
    		lv_update_bar_schedule(datosApp, true);
    	}

    }





}


void display_status_application(DATOS_APLICACION *datosApp, cJSON *respuesta) {

	ESP_LOGI(TAG, ""TRAZAR"display_status_application", INFOTRAZA);
	//gpio_rele_in();
    cJSON_AddNumberToObject(respuesta, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
    cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
    cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
    cJSON_AddNumberToObject(respuesta, TEMPERATURA, datosApp->termostato.tempActual);
    cJSON_AddNumberToObject(respuesta, HUMEDAD, datosApp->termostato.humedad);
    cJSON_AddNumberToObject(respuesta, UMBRAL_TEMPERATURA, datosApp->termostato.tempUmbral);
    cJSON_AddNumberToObject(respuesta, MARGEN_TEMPERATURA, datosApp->termostato.margenTemperatura);
    cJSON_AddNumberToObject(respuesta, INTERVALO_LECTURA, datosApp->termostato.read_interval);
    cJSON_AddNumberToObject(respuesta, INTERVALO_REINTENTOS, datosApp->termostato.retry_interval);
    cJSON_AddNumberToObject(respuesta, REINTENTOS_LECTURA, datosApp->termostato.reintentosLectura);
    cJSON_AddNumberToObject(respuesta, CALIBRADO, datosApp->termostato.calibrado);
    cJSON_AddBoolToObject(respuesta, MASTER, datosApp->termostato.master);
    cJSON_AddStringToObject(respuesta, SENSOR_REMOTO, datosApp->termostato.sensor_remoto);
    cJSON_AddNumberToObject(respuesta, UMBRAL_DEFECTO, datosApp->termostato.tempUmbralDefecto);


    //appUser_parametrosAplicacion(datosApp, respuesta);
    escribir_programa_actual(datosApp, respuesta);
    codigoRespuesta(respuesta,RESP_OK);



}


void display_act_remote_relay(DATOS_APLICACION *datosApp, cJSON *respuesta) {

	ESP_LOGI(TAG, ""TRAZAR"display_act_remote_relay", INFOTRAZA);
    cJSON_AddNumberToObject(respuesta, DLG_COD_RESPUESTA, DLG_OK_CODE);
    cJSON_AddNumberToObject(respuesta, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
    cJSON_AddNumberToObject(respuesta, DEVICE_STATE, datosApp->datosGenerales->estadoApp);
    cJSON_AddNumberToObject(respuesta, PROGRAMMER_STATE, datosApp->datosGenerales->estadoProgramacion);
    cJSON_AddNumberToObject(respuesta, TEMPERATURA, datosApp->termostato.tempActual);
    cJSON_AddNumberToObject(respuesta, HUMEDAD, datosApp->termostato.humedad);
    cJSON_AddNumberToObject(respuesta, UMBRAL_TEMPERATURA, datosApp->termostato.tempUmbral);
    escribir_programa_actual(datosApp, respuesta);
    codigoRespuesta(respuesta,DLG_OK_CODE);
}


bool modify_threshold_temperature(cJSON *peticion, DATOS_APLICACION *datosApp, cJSON *respuesta) {

    cJSON *nodo = NULL;
    cJSON *campo = NULL;



    ESP_LOGI(TAG, ""TRAZAR"modify_threshold_temperature", INFOTRAZA);
    nodo = cJSON_GetObjectItem(peticion, MODIFICAR_APP);
   if(nodo == NULL) {
       return NULL;
   }

    printf("modify_threshold_temperature-->comienzo\n");
    campo = cJSON_GetObjectItem(nodo, UMBRAL_TEMPERATURA);
       if((campo != NULL) && (campo->type == cJSON_Number)) {
           printf("modificando umbral\n");
           datosApp->termostato.tempUmbral = campo->valuedouble;

           lv_update_threshold(datosApp, true);
           thermostat_action(datosApp);
           cJSON_AddNumberToObject(respuesta, UMBRAL_TEMPERATURA, datosApp->termostato.tempUmbral);
           cJSON_AddNumberToObject(respuesta, APP_COMAND_ESTADO_RELE, gpio_get_level(CONFIG_GPIO_PIN_RELE));
           codigoRespuesta(respuesta, RESP_OK);
       } else {
           codigoRespuesta(respuesta, RESP_NOK);
       }
    return true;
}





esp_err_t select_temperature_sensor(cJSON *peticion, DATOS_APLICACION *datosApp, cJSON *respuesta) {

	cJSON *nodo = NULL;
    bool master = true;

    ESP_LOGI(TAG, ""TRAZAR"select_temperature_sensor", INFOTRAZA);
    nodo = cJSON_GetObjectItem(peticion, MODIFICAR_SENSOR_TEMPERATURA);
   if(nodo == NULL) {
	   ESP_LOGE(TAG, ""TRAZAR" NO SE ENCUENTRA EL PATRON DE SENSOR EN EL COMANDO", INFOTRAZA);
	   codigoRespuesta(respuesta, RESP_NOK);
	   return ESP_FAIL;
   }



   if (extraer_dato_uint8(nodo,  MASTER, (uint8_t*) &master) != ESP_OK) {
	   ESP_LOGE(TAG, ""TRAZAR" NO VIENE EL CAMPO MASTER EN LA PETICION", INFOTRAZA);
	   codigoRespuesta(respuesta, RESP_NOK);
	   return ESP_FAIL;
   }

   if (master) {
	   ESP_LOGI(TAG, ""TRAZAR" Se modifica el sensor para que el master sea el dispositivo", INFOTRAZA);
	   memset(datosApp->termostato.sensor_remoto, 0,sizeof(datosApp->termostato.sensor_remoto));
	   unsubscribe_topic(datosApp, CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE);
   } else {
	   ESP_LOGE(TAG, ""TRAZAR"ante: "CONFIG_UINT32_FORMAT"\n", INFOTRAZA, esp_get_free_heap_size());
	   //strcpy(datosApp->termostato.sensor_remoto, cJSON_GetObjectItem(nodo, SENSOR_REMOTO)->valuestring);
	   extraer_dato_string(nodo, SENSOR_REMOTO, datosApp->termostato.sensor_remoto);
	   ESP_LOGE(TAG, ""TRAZAR"despues: "CONFIG_UINT32_FORMAT"\n", INFOTRAZA, esp_get_free_heap_size());
	   ESP_LOGW(TAG, ""TRAZAR" Se selecciona el sensor remoto a :%s", INFOTRAZA, datosApp->termostato.sensor_remoto);
	   set_topic_remote_sensor(datosApp, datosApp->termostato.sensor_remoto);
	   subscribe_topic(datosApp, datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].subscribe);

   }
   datosApp->termostato.master = master;
   salvar_configuracion_general(datosApp);
   codigoRespuesta(respuesta, RESP_OK);

	return ESP_OK;
}


esp_err_t appuser_set_command_application(cJSON *peticion, int nComando, DATOS_APLICACION *datosApp, cJSON *respuesta) {

	ESP_LOGI(TAG, ""TRAZAR"appuser_set_command_application", INFOTRAZA);
    switch(nComando) {
        case OP_RELAY:
            relay_operation(datosApp, MANUAL, INDETERMINADO);
            display_act_remote_relay(datosApp, respuesta);
            break;

        case STATUS_DEVICE:
            display_status_application(datosApp, respuesta);
            break;
        case MODIFY_THRESHOLD_TEMPERATURE:
        	modify_threshold_temperature(peticion, datosApp, respuesta);

        	break;
        case SELECT_SENSOR_TEMPERATURE:
        	select_temperature_sensor(peticion, datosApp, respuesta);
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

	lv_update_bar_schedule(datosApp, false);

	switch (datosApp->datosGenerales->estadoApp) {

	case NO_PROGRAM:
	case NORMAL_AUTO:
	case NORMAL_AUTOMAN:
		datosApp->termostato.tempUmbral = datosApp->termostato.tempUmbralDefecto;
		lv_update_threshold(datosApp, true);
		break;

	case CHECK_PROGRAMS:
		datosApp->termostato.tempUmbral = datosApp->termostato.tempUmbralDefecto;
		//change_status_application(datosApp, NORMAL_AUTO);
		break;

	default:
		break;
	}




}



void change_threshold(void *arg) {


	DATOS_APLICACION *datosApp;
	datosApp = (DATOS_APLICACION*) arg;
	thermostat_action(datosApp);
	lv_update_threshold(datosApp, true);
	send_spontaneous_report(datosApp, CAMBIO_UMBRAL_TEMPERATURA);


}




void appuser_received_local_event(DATOS_APLICACION *datosApp, EVENT_DEVICE event) {



	ESP_LOGI(TAG, ""TRAZAR"appuser_received_local_event: recibido :%s", INFOTRAZA, local_event_2_mnemonic(event));
	static esp_timer_handle_t temporizador_duracion;

    const esp_timer_create_args_t change_threshold_timer_args = {
            .callback = &change_threshold,
            /* name is optional, but may help identify the timer when debugging */
            .name = "up threshold",
			.arg = (void*) datosApp
    };


	switch (event) {

	case EVENT_UP_THRESHOLD:
		ESP_LOGE(TAG,""TRAZAR"PROCESAMOS EVENT_UP_THRESHOLD", INFOTRAZA);
		datosApp->termostato.tempUmbral += datosApp->termostato.incdec;
		lv_update_threshold(datosApp, false);
		if (esp_timer_is_active(temporizador_duracion)) {
			esp_timer_stop(temporizador_duracion);
			esp_timer_delete(temporizador_duracion);
			ESP_LOGI(TAG, ""TRAZAR"timer cancelado", INFOTRAZA);
		}
	    ESP_ERROR_CHECK(esp_timer_create(&change_threshold_timer_args, &temporizador_duracion));
	    ESP_ERROR_CHECK(esp_timer_start_once(temporizador_duracion, (3000000)));

		break;
	case EVENT_DOWN_THRESHOLD:
		datosApp->termostato.tempUmbral -= datosApp->termostato.incdec;
		lv_update_threshold(datosApp, false);
		if (esp_timer_is_active(temporizador_duracion)) {
			esp_timer_stop(temporizador_duracion);
			esp_timer_delete(temporizador_duracion);
			ESP_LOGI(TAG, ""TRAZAR"timer cancelado", INFOTRAZA);
		}
	    ESP_ERROR_CHECK(esp_timer_create(&change_threshold_timer_args, &temporizador_duracion));
	    ESP_ERROR_CHECK(esp_timer_start_once(temporizador_duracion, (3000000)));
		break;

	default:
	case EVENT_ANSWER_TEMPERATURE:
		update_thermostat_device(datosApp);
		break;


	}

}




void appuser_notify_alarm_on_device(DATOS_APLICACION *datosApp, ALARM_TYPE alarm) {


	ESP_LOGE(TAG, ""TRAZAR"ALARMA %s en estado ON", INFOTRAZA, datosApp->alarmas[alarm].nemonico);
	switch (alarm) {

	case ALARM_REMOTE_DEVICE:
		lv_update_alarm_device(datosApp);
		break;
	default:
		break;
	}







}


void appuser_notify_alarm_off_device(DATOS_APLICACION *datosApp, ALARM_TYPE alarm) {


	ESP_LOGE(TAG, ""TRAZAR"ALARMA %s en estado OFF", INFOTRAZA, datosApp->alarmas[alarm].nemonico);
	switch (alarm) {

	case ALARM_REMOTE_DEVICE:
		lv_update_alarm_device(datosApp);
		break;
	default:
		break;
	}

}

void appuser_notify_error_remote_device(DATOS_APLICACION *datosApp) {


	EVENT_DEVICE event = EVENT_NONE;
	ESP_LOGI(TAG, ""TRAZAR"LEEMOS EN LOCAL PORQUE NO RESPONDE EL DISPOSITIVO REMOTO", INFOTRAZA);

	event = reading_local_temperature(datosApp);


	switch(event) {

	case EVENT_ANSWER_TEMPERATURE:
		send_event_device(__func__,EVENT_ANSWER_TEMPERATURE);
		//update_thermostat_device(datosApp);
		break;
	case EVENT_ERROR_READ_LOCAL_TEMPERATURE:
		send_event_device(__func__,EVENT_ERROR_READ_LOCAL_TEMPERATURE);
		break;

	default:
		break;

	}




}

void appuser_notify_smartconfig_end(DATOS_APLICACION *datosApp) {


	lv_screen_thermostat(datosApp);


}

void appuser_notify_error_smartconfig(DATOS_APLICACION *datosApp) {

	ESP_LOGE(TAG, ""TRAZAR"Senalizamos el error en smartconfig", INFOTRAZA);


}


