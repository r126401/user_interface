/*
 * local_events.c
 *
 *  Created on: Oct 23, 2023
 *      Author: t126401
 */


#include "local_events_device.h"
#include "code_application.h"
#include "common_data.h"
#include "esp_timer.h"

#include "user_interface.h"
#include "esp_err.h"
#include "logging.h"
#include "events_device.h"
#include "alarmas.h"
#include "driver/gpio.h"
#include "conexiones.h"




static const char *TAG = "LOCAL_EVENTS_DEVICE";
static esp_timer_handle_t timer_push;
#define LARGE_INTERVAL 3
#define VERY_LARGE_INTERVAL 5



char* local_event_2_mnemonic(EVENT_DEVICE event) {

	static char mnemonic[30] = {0};


	switch(event) {

	case EVENT_NONE:
		strcpy(mnemonic, "EVENT_NONE");
		break;
	case EVENT_SHORT_PUSH_BUTTON:
		strcpy(mnemonic, "EVENT_SHORT_PUSH_BUTTON");

		break;
	case EVENT_LARGE_PUSH_BUTTON:
		strcpy(mnemonic, "EVENT_LARGE_PUSH_BUTTON");
		break;

	case EVENT_VERY_LARGE_PUSH_BUTTON:
		strcpy(mnemonic, "EVENT_VERY_LARGE_PUSH_BUTTON");
		break;
	case EVENT_OP_LOCAL_RELAY:
		strcpy(mnemonic, "EVENT_OP_LOCAL_RELAY");

		break;
	case EVENT_OP_REMOTE_RELAY:
		strcpy(mnemonic, "EVENT_OP_REMOTE_RELAY");
		break;



	}

	return mnemonic;


}


void extra_time_push(void *arg) {

	bool *push = (bool*) arg;
	push = false;

	ESP_LOGI(TAG, ""TRAZAR"PUSH PASADO A FALSE", INFOTRAZA);



}




void process_event_short_push_button(DATOS_APLICACION *datosApp) {



	static uint8_t seconds = 0;
	static bool push = false;
	static esp_timer_handle_t release_timer;
	ESP_LOGI(TAG, ""TRAZAR"process_event_short_push_button", INFOTRAZA);
    const esp_timer_create_args_t repeater_timer_args = {
            .callback = &process_event_short_push_button,
            /* name is optional, but may help identify the timer when debugging */
            .name = "repeater manage button",
			.arg = (DATOS_APLICACION*) datosApp
    };

    const esp_timer_create_args_t release_timer_args = {
            .callback = &extra_time_push,
            /* name is optional, but may help identify the timer when debugging */
            .name = "repeater manage button",
			.arg = (bool*) push
    };



    if (!push) {
    	ESP_LOGW(TAG, ""TRAZAR"Se pulsa el boton", INFOTRAZA);
    	push = true;
    	esp_timer_create(&repeater_timer_args, &timer_push);
    	esp_timer_start_once(timer_push, 1000000);
    } else {
    	if (gpio_get_level(CONFIG_GPIO_PIN_BOTON) == 0) {
    		seconds++;
    		ESP_LOGW(TAG, ""TRAZAR"Continua pulsado... %d segundos", INFOTRAZA, seconds);
        	esp_timer_create(&repeater_timer_args, &timer_push);
        	esp_timer_start_once(timer_push, 1000000);
        	if (seconds > LARGE_INTERVAL) {
        		gpio_set_level(CONFIG_GPIO_PIN_LED, ON);
        	}


    	} else {
    		ESP_LOGW(TAG, ""TRAZAR"Boton liberado... %d segundos", INFOTRAZA, seconds);
    		esp_timer_stop(timer_push);
    		esp_timer_delete(timer_push);

    		if(seconds > VERY_LARGE_INTERVAL)  {
    			ESP_LOGW(TAG, ""TRAZAR"PULSACION MUY LARGA!!!!", INFOTRAZA);
    			send_event_device(__func__, EVENT_VERY_LARGE_PUSH_BUTTON);

    		} else {
        		if (seconds > LARGE_INTERVAL) {
        			ESP_LOGW(TAG, ""TRAZAR"PULSACION LARGA!!!!", INFOTRAZA);
        			send_event_device(__func__, EVENT_LARGE_PUSH_BUTTON);

        		} else {
        			ESP_LOGW(TAG, ""TRAZAR"PULSACION CORTA!!!!", INFOTRAZA);
        			send_event_device(__func__, EVENT_OP_LOCAL_RELAY);

        		}
    		}
    		seconds=0;
    		esp_timer_create(&release_timer_args, &release_timer);
    		esp_timer_start_once(release_timer, 250000);
    		//push = false;

    	}

    }


}

void process_event_large_push_button(DATOS_APLICACION *datosApp) {

	if (tcpip_adapter_is_netif_up(ESP_IF_WIFI_STA) == true) {
		esp_restart();
	} else {
		send_event(__func__, EVENT_FACTORY);
	}


}

void process_event_very_large_push_button(DATOS_APLICACION *datosApp) {


	reinicio_fabrica(datosApp);
	send_event(__func__, EVENT_FACTORY);


}



void process_event_op_local_relay(DATOS_APLICACION* datosApp) {

	relay_operation(datosApp, MANUAL, INDETERMINADO);
	send_spontaneous_report(datosApp, OP_LOCAL_RELAY);
}



void received_local_event(DATOS_APLICACION *datosApp, EVENT_DEVICE event) {

	ESP_LOGI(TAG, ""TRAZAR"Recibido local event", INFOTRAZA);


	switch (event) {

	case EVENT_NONE:

		break;
	case EVENT_SHORT_PUSH_BUTTON:
		process_event_short_push_button(datosApp);
		break;
	case EVENT_LARGE_PUSH_BUTTON:
		process_event_large_push_button(datosApp);
		break;
	case EVENT_VERY_LARGE_PUSH_BUTTON:
		process_event_very_large_push_button(datosApp);
		break;
	case EVENT_OP_LOCAL_RELAY:
		process_event_op_local_relay(datosApp);


		break;
	case EVENT_OP_REMOTE_RELAY:

		break;

	}




}
