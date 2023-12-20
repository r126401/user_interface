/*
 * local_events.c
 *
 *  Created on: Oct 23, 2023
 *      Author: t126401
 */


#include "local_events_device.h"
#include "common_data.h"
#include "esp_timer.h"

#include "user_interface.h"
#include "esp_err.h"
#include "logging.h"
#include "events_device.h"
#include "alarmas.h"




static const char *TAG = "LOCAL_EVENTS_DEVICE";

#define TIMEOUT_REQUEST_REMOTE_TEMPERATURE 5
#define NUM_FAILS 5
static esp_timer_handle_t timer_request_remote_temperature;
const esp_timer_create_args_t timer_remote_read_args;


void event_handler_request_remote_temperature(void *arg) {


	send_event_device(__func__ ,EVENT_TIMEOUT_REMOTE_TEMPERATURE);


}


void process_local_event_request_temperature(DATOS_APLICACION *datosApp) {


}






void process_local_event_timeout_reading_temperature(DATOS_APLICACION *datosApp, bool reset_counter) {

	static int error_counter = 0;


	ESP_LOGW(TAG, ""TRAZAR"Reiniciamos el contador de fallos reset es %d y error_counter: %d", INFOTRAZA, reset_counter, error_counter);
	if (reset_counter) {
		error_counter = 0;
		ESP_LOGW(TAG, ""TRAZAR"Sensor %d se ha recuperado antes de llegar al umbral de fallo ", INFOTRAZA, datosApp->termostato.master);
		send_event_device(__func__,EVENT_REMOTE_DEVICE_OK);
		return;
	} else {
		ESP_LOGW(TAG,""TRAZAR" CONTADOR DE FALLOS %d", INFOTRAZA, error_counter);
		if (error_counter >= NUM_FAILS) {
			if (datosApp->termostato.master) {
				send_event(__func__,EVENT_ERROR_DEVICE);
			}else {
				send_event(__func__,EVENT_ERROR_REMOTE_DEVICE);
			}

			ESP_LOGW(TAG, ""TRAZAR"Sensor escalvo/master (0/1) %d en fallo entramos en politica de reintentos ", INFOTRAZA, datosApp->termostato.master);


		}
		error_counter++;
	}
}




void process_local_event_answer_temperature(DATOS_APLICACION *datosApp) {


	ESP_LOGI(TAG, ""TRAZAR"process_local_event_answer_temperature", INFOTRAZA);

	if (!datosApp->termostato.master) {
		if (esp_timer_is_active(timer_request_remote_temperature)) {
			ESP_LOGI(TAG, ""TRAZAR"se cancela temporizador", INFOTRAZA);
			esp_timer_delete(timer_request_remote_temperature);
		}
	}
	ESP_LOGW(TAG, ""TRAZAR"Reiniciamos el contador de fallos", INFOTRAZA);
	process_local_event_timeout_reading_temperature(datosApp, true);
	send_event(__func__,EVENT_DEVICE_OK);
	appuser_received_local_event(datosApp, EVENT_ANSWER_TEMPERATURE);




}

void process_local_event_waiting_response_temperature(DATOS_APLICACION *datosApp) {


    const esp_timer_create_args_t timer_remote_read_args = {
    		.callback = &event_handler_request_remote_temperature,
			.name = "timer remote read",
			.arg = (void*) datosApp
    };


	//process_local_event_answer_temperature(datosApp);
	ESP_LOGI(TAG, ""TRAZAR"ESPERANDO RESPUESTA DEL DISPOSITIVO REMOTO", INFOTRAZA);
    ESP_ERROR_CHECK(esp_timer_create(&timer_remote_read_args, &timer_request_remote_temperature));
    ESP_ERROR_CHECK(esp_timer_start_once(timer_request_remote_temperature, TIMEOUT_REQUEST_REMOTE_TEMPERATURE * 1000000));

}



void received_local_event(DATOS_APLICACION *datosApp, EVENT_DEVICE event) {

	switch (event) {

	case EVENT_REQUEST_TEMPERATURE:
		process_local_event_request_temperature(datosApp);
		break;
	case EVENT_ANSWER_TEMPERATURE:
		process_local_event_answer_temperature(datosApp);
		break;
	case EVENT_TIMEOUT_REMOTE_TEMPERATURE:
		process_local_event_timeout_reading_temperature(datosApp, false);
		break;

	case EVENT_UP_THRESHOLD:
		appuser_received_local_event(datosApp, event);
		break;
	case EVENT_DOWN_THRESHOLD:
		appuser_received_local_event(datosApp, event);
		break;

	case EVENT_ERROR_READ_LOCAL_TEMPERATURE:
		process_local_event_timeout_reading_temperature(datosApp, false);
		break;

	case EVENT_WAITING_RESPONSE_TEMPERATURE:
		process_local_event_waiting_response_temperature(datosApp);
		break;
	default:
		break;

	}




}
