#include "ch.h"
#include "hal.h"
#include <stdint.h>
#include <audio/play_melody.h>
#include <msgbus/messagebus.h>
#include <motors.h>
#include <sensors/proximity.h>


#include "sound.h"
#include "tempo.h"

#define CM_TO_STEPS(cm) (1000*(cm)/13) //converts distances for e-puck2 motors
#define DURATION 150 //ms

static void update_melody(messagebus_topic_t *proximity_topic, uint16_t* note);

static THD_WORKING_AREA(waSound, 256);
static THD_FUNCTION(Sound, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    uint16_t note = NOTE_C1;
    uint16_t duration = DURATION;
 //   melody_t melody = {&note, &duration, 1};

    int16_t default_speed = 0;
	messagebus_topic_t *proximity_topic = messagebus_find_topic_blocking(&bus, "/proximity");
	static uint8_t running = 0;

    while(1)
    {
        get_tempo(&default_speed, proximity_topic);
        update_melody(proximity_topic, &note);
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


        playNote(note, duration); //!\\ blocks the thread
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
#define SCALE_SIZE 8
static void update_melody(messagebus_topic_t *proximity_topic, uint16_t* note)
{
	uint16_t c_major_scale[SCALE_SIZE] = {NOTE_C3, NOTE_D3, NOTE_E3, NOTE_F3, NOTE_G3, NOTE_A3, NOTE_B3, NOTE_C4};
	proximity_msg_t prox_buf;

	if(messagebus_topic_read(proximity_topic, &prox_buf, sizeof(proximity_msg_t)))
	{
		for(uint8_t i=0; i < SCALE_SIZE; i++)
		{
			if(prox_buf.delta[i] > 250)
			{
				*note = c_major_scale[i];
				i = SCALE_SIZE;
			}
		}
	}
}
