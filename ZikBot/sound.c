#include "ch.h"
#include "hal.h"
#include <stdint.h>
#include <audio/play_melody.h>
#include <msgbus/messagebus.h>
#include <motors.h>
#include <sensors/proximity.h>

//debug purposes
#include "leds.h"
#include <chprintf.h>
#include <move.h>

#include "sound.h"
#include "process_image.h"

#define CM_TO_STEPS(cm) (1000*(cm)/13) //converts distances for e-puck2 motors
#define MELODY_LENGTH 1
#define NOTE_TEMPO 8 //s^(-1)
#define NOTE_DURATION 125	//ms

static uint16_t get_note(void);

static THD_WORKING_AREA(waSound, 256);
static THD_FUNCTION(Sound, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

//    int16_t default_speed = 0;
//	messagebus_topic_t *proximity_topic = messagebus_find_topic_blocking(&bus, "/proximity");
//	static uint8_t running = 0;
//	systime_t time;

    while(1)
    {
//		time = chVTGetSystemTime();
    	playNote(get_note(), NOTE_DURATION); //chut

//		chThdSleepUntilWindowed(time, time + MS2ST(200));
    }
}

void sound_start(void){
	chThdCreateStatic(waSound, sizeof(waSound), (NORMALPRIO), Sound, NULL);
}


#define NB_NOTES 8 //
#define REL_POS_THRESHOLD (FULL_SCALE/NB_NOTES)
static const uint16_t notes[NB_NOTES] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
//static const uint16_t notes[NB_NOTES] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_B4, NOTE_D5};

static uint16_t get_note(void)
{
	if(note_rel_pos == 0)
		return 0;
    for(uint16_t i=0; i < NB_NOTES; i++)
    {
      	if(note_rel_pos < (i+1)*REL_POS_THRESHOLD)
      	{
        	return notes[i];
		}
    }
    return 0;
}
