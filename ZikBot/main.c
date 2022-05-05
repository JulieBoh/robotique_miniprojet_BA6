#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <chprintf.h>
#include <main.h>
#include <motors.h>
#include <camera/po8030.h>
#include <sensors/proximity.h>
#include <audio/audio_thread.h>
#include <audio/play_melody.h>
#include <leds.h>

//#include <pi_regulator.h>
//#include <process_image.h>
#include <tempo.h>
#include <sound.h>

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
/** END OF DEBUG FUCTIONS **/


// bus declaration
messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);


// MAIN //
int main(void)
{
	//init hal, ChibiOS & MPU
    halInit();
    chSysInit();
    mpu_init();

    // bus init
    messagebus_init(&bus, &bus_lock, &bus_condvar);

    /*debug purposes*/
    //starts the serial communication
    serial_start();
    //start the USB communication
    usb_start();

	//LEDS: light under mirror
	set_body_led(1);

    //starts the camera
//    dcmi_start();
//	po8030_start();
	//inits the motors
	motors_init();

	//IR init
	proximity_start();
	calibrate_ir();

	//sound init
	dac_start();
	playMelodyStart();
	//starts the threads
	sound_start();
//	process_image_start();

	//static int16_t default_speed = 0;

    /* Infinite loop. */
    while (1) {
    	//waits 1 second
        chThdSleepMilliseconds(500);
    }
}


// SECURITY purposes //
#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
