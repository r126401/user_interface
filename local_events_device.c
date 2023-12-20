/*
 * local_events.c
 *
 *  Created on: Oct 23, 2023
 *      Author: t126401
 */


#include "local_events_device.h"
#include "common_data.h"
#include "esp_timer.h"

#include "interfaz_usuario.h"
#include "esp_err.h"
#include "logging.h"
#include "events_device.h"
#include "alarmas.h"




static const char *TAG = "LOCAL_EVENTS_DEVICE";









void received_local_event(DATOS_APLICACION *datosApp, EVENT_DEVICE event) {

	switch (event) {

	default:
		break;

	}




}
