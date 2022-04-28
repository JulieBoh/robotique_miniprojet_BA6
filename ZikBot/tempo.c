#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <tempo.h>
#include <process_image.h>

static THD_WORKING_AREA(waTempo, 256);
static THD_FUNCTION(Tempo, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
        /*TO BE DONE*/
        // Read IR
        // Compute tempo
        // send new tempo to motor

        //chThdSleepUntilWindowed(time, time + MS2ST(10));
        chThdSleepUntilWindowed(10, 11);
    }
}

void tempo_start(void){
	chThdCreateStatic(waTempo, sizeof(waTempo), NORMALPRIO, Tempo, NULL);
}
