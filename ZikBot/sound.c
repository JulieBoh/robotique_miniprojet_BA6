#include "ch.h"
#include "hal.h"
#include <stdint.h>
#include <audio/play_melody.h>
#include "audio/audio_thread.h"
#include <msgbus/messagebus.h>
#include <motors.h>
#include <sensors/proximity.h>

//debug purposes
#include "leds.h"
#include <chprintf.h>

#include "main.h"
#include "sound.h"
#include "tempo.h"
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

    while(1)
    {
     	set_led(LED3, 2);
//		time = chVTGetSystemTime();
//    	playNote(get_note(), NOTE_DURATION); //chut
     	uint16_t note = get_note();
     	if(note != 0){
     			dac_play(note);
     		}
		chBSemWait(&note_ready_sem);
     	dac_stop();
//		chThdSleepUntilWindowed(time, time + MS2ST(200));
     }
}

void sound_start(void){
	chThdCreateStatic(waSound, sizeof(waSound), (NORMALPRIO+1), Sound, NULL);
}


#define SCALE_SIZE 5
#define REL_POS_THRESHOLD (100/SCALE_SIZE)
//static const uint16_t c_major_scale[SCALE_SIZE] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
static const uint16_t c_major_scale[SCALE_SIZE] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_B4, NOTE_D4};

static uint16_t get_note(void)
{
	uint8_t rel_pos = get_rel_pos();

	if(rel_pos == 0)
		return 0;
    for(uint8_t i=0; i < SCALE_SIZE; i++){
    	if(rel_pos < (i+1)*REL_POS_THRESHOLD){
        	return c_major_scale[i];
		}
    }
    return 0;//in case we're out of luck
}
