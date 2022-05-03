#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <motors.h>
#include <camera/po8030.h>
#include <chprintf.h>
#include <leds.h>


#include <tempo.h>
#include <process_image.h>

// COMMUNICATION //
void SendUint8ToComputer(uint8_t* data, uint16_t size) 
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

// MAIN //

int main(void)
{
	//init hal, ChibiOS & MPU
    halInit();
    chSysInit();
    mpu_init();

    /*debug purposes*/
    //starts the serial communication
    serial_start();
    //start the USB communication
    usb_start();

	//LEDS: light under mirror
	set_body_led(1);

    //starts the camera
    dcmi_start();
	po8030_start();
	//inits the motors
	motors_init();

	/* TO BE DONE -> Setup Routine*/
		//read IR
		//set tempo
		//affichage statut

	//starts the threads
	tempo_start();
	process_image_start();



    /* Infinite loop. */
    while (1) {
    	//waits 1 second
        chThdSleepMilliseconds(1000);
    }
}


// SECURITY purposes //
#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
