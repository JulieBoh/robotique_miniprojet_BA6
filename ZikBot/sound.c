#include "ch.h"
#include "hal.h"
#include <stdint.h>
#include <audio/play_melody.h>
#include <msgbus/messagebus.h>
#include <motors.h>
#include <sensors/proximity.h>

//debug purposes
#include <chprintf.h>

#include "sound.h"
#include "tempo.h"

#define CM_TO_STEPS(cm) (1000*(cm)/13) //converts distances for e-puck2 motors
#define MELODY_LENGTH 1
#define NOTE_TEMPO 8 //s^(-1)
#define NOTE_DURATION 125	//ms

static uint16_t get_note(messagebus_topic_t *proximity_topic);

static THD_WORKING_AREA(waSound, 256);
static THD_FUNCTION(Sound, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    int16_t default_speed = 0;
	messagebus_topic_t *proximity_topic = messagebus_find_topic_blocking(&bus, "/proximity");
	static uint8_t running = 0;

    while(1)
    {
    	playNote(get_note(proximity_topic), NOTE_DURATION);
//		WORKING : CHANGES STANDARD SPEED OF THE ROBOT
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

    }
}

void sound_start(void){
	chThdCreateStatic(waSound, sizeof(waSound), NORMALPRIO, Sound, NULL);
}

//void sound_test(int16_t* default_speed)
//{
//	if(*default_speed % 2 == 0)
//	{
//		playMelody(MARIO_DEATH, ML_WAIT_AND_CHANGE, NULL);
//	}
//	else
//	{
//		playMelody(STARWARS, ML_WAIT_AND_CHANGE, NULL);
//	}
//}

#define SCALE_SIZE 8
static const uint16_t c_major_scale[SCALE_SIZE] = {NOTE_C3, NOTE_D3, NOTE_E3, NOTE_F3, NOTE_G3, NOTE_A3, NOTE_B3, NOTE_C4};

static uint16_t get_note(messagebus_topic_t *proximity_topic)
{
	proximity_msg_t prox_buf;

	if(messagebus_topic_read(proximity_topic, &prox_buf, sizeof(proximity_msg_t)))
	{
		for(uint8_t i=0; i < SCALE_SIZE; i++)
		{
			if(prox_buf.delta[i] > 250)
			{
				return c_major_scale[i];
			}
		}
	}
	return 0;
}
