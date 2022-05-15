#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <main.h>
#include <motors.h>
#include <camera/po8030.h>
#include <sensors/proximity.h>
#include <audio/audio_thread.h>
#include <leds.h>

#include "move.h"
#include "process_image.h"
#include "sound.h"

// bus declaration
messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

BSEMAPHORE_DECL(note_ready_sem, FALSE);

// MAIN //
int main(void)
{
	//init hal, ChibiOS & MPU
    halInit();
    chSysInit();
    mpu_init();

    // bus init
    messagebus_init(&bus, &bus_lock, &bus_condvar);

	//LEDS: light under mirror
	set_body_led(1);

    //starts the camera
    dcmi_start();
	po8030_start();

	//IR init
	proximity_start();
	calibrate_ir();

	//sound init
	dac_start();

	//move module start
	motors_init();
	move_init();

	//starts the threads
	process_image_start();
	sound_start();

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
