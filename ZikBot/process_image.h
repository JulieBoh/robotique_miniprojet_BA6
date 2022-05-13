#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define MAX_LINE_NBR 3 // MAX_LINE_NBR - 2 (margins) = nbr of notes permitted at the same time music arrangement (here no arrangement permitted)

//extern uint8_t note_rel_pos; //[%]


void process_image_start(void);
uint16_t * image_analyse(const uint8_t* image);
void outlier_detection(uint16_t* lines_position, uint16_t (*lines_pos_history)[MAX_LINE_NBR], uint8_t line_nbr);
void sendnote2buzzer(uint16_t* pos_ptr);
void path_processing(uint16_t* pos_ptr);
uint8_t get_rel_pos(void);

#endif /* PROCESS_IMAGE_H */
