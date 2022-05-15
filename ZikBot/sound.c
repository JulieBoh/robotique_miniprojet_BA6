#include "ch.h"
#include "hal.h"
#include <stdint.h>
#include <audio/play_melody.h>

#include "main.h"
#include "sound.h"
#include "process_image.h"

#define NOTE_DURATION 125
#define NB_NOTES 8 //
#define REL_POS_THRESHOLD (FULL_SCALE/NB_NOTES)
static const uint16_t notes[NB_NOTES] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
//static const uint16_t notes[NB_NOTES] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_B4, NOTE_D5};

static uint16_t get_note(void){
	if(note_rel_pos == 0)
		return 0;
    for(uint16_t i=0; i < NB_NOTES; i++){
      	if(note_rel_pos < (i+1)*REL_POS_THRESHOLD)
        	return notes[i];
    }
    return 0; // in case we're out of luck
}

static THD_WORKING_AREA(waSound, 256);
static THD_FUNCTION(Sound, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
    	playNote(get_note(), NOTE_DURATION);
    }
}

void sound_start(void){
	chThdCreateStatic(waSound, sizeof(waSound), (NORMALPRIO), Sound, NULL);
}
