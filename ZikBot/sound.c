#include "ch.h"
#include "hal.h"
#include <stdint.h>
#include <audio/play_melody.h>
#include <msgbus/messagebus.h>
#include <motors.h>

#include "sound.h"
#include "tempo.h"

#define CM_TO_STEPS(cm) (1000*(cm)/13) //converts distances for e-puck2 motors

static THD_WORKING_AREA(waSound, 256);
static THD_FUNCTION(Sound, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    //systime_t time;
    int16_t default_speed = 0;
	messagebus_topic_t *proximity_topic = messagebus_find_topic_blocking(&bus, "/proximity");
	static uint8_t running = 0;

    while(1){
        /*TO BE DONE*/
        // Read IR
        // Compute tempo
        // send new tempo to motor

        //chThdSleepUntilWindowed(time, time + MS2ST(10));
    	/*if(*default_speed % 2 == 0)
    		{
    			playMelody(MARIO_DEATH, ML_WAIT_AND_CHANGE, NULL);
    		}
    		else
    		{
    			playMelody(STARWARS, ML_WAIT_AND_CHANGE, NULL);
    		}
*/
        get_tempo(&default_speed, proximity_topic);
        running = 1;
        if(running)
        {
        	if(default_speed > 14)
        		default_speed = 14;
        	if(default_speed < -14)
        		default_speed = -14;
        	left_motor_set_speed(CM_TO_STEPS(default_speed));
        	right_motor_set_speed(CM_TO_STEPS(default_speed));
        }
        chThdSleep(100);
    }
}

void sound_start(void){
	chThdCreateStatic(waSound, sizeof(waSound), NORMALPRIO, Sound, NULL);
}

void sound_test(int16_t* default_speed)
{
	if(*default_speed % 2 == 0)
	{
		playMelody(MARIO_DEATH, ML_WAIT_AND_CHANGE, NULL);
	}
	else
	{
		playMelody(STARWARS, ML_WAIT_AND_CHANGE, NULL);
	}
}
