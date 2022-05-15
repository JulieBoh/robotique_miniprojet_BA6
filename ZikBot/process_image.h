#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define MAX_LINE_NBR 3 // MAX_LINE_NBR - 2 (margins) = nbr of notes permitted at the same time music arrangement (here no arrangement permitted)
#define FULL_SCALE 100 //
extern uint16_t note_rel_pos; //[%]

void process_image_start(void);

#endif /* PROCESS_IMAGE_H */
