#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define MAX_LINE_NBR 3 // MAX_LINE_NBR - 2 (margins) = nbr of notes permitted at the same time music arrangement (here no arrangement permitted)
//TO DO: ajouter MAX_DETECTABLE_LINE_NBR 10 pour pouvoir avoir des nbr=5 p.ex qui font une erreur.

extern uint8_t note_rel_pos; //[%]


void process_image_start(void);
uint16_t * image_analyse(const uint8_t* image, bool is_reading_bottom);
void outlier_detection(uint16_t* lines_position, uint16_t (*lines_pos_history)[MAX_LINE_NBR], uint8_t line_nbr);
void sendnote2buzzer(uint16_t* bottom_pos);
int16_t path_processing(uint16_t* bottom_pos, uint16_t* top_pos);

#endif /* PROCESS_IMAGE_H */
