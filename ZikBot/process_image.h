#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define MAX_LINE_NBR 3 // MAX_LINE_NBR - 2 (margins) = nbr of notes permitted at the same time music arrangement (here no arrangement permitted)

extern uint8_t note_rel_pos; //[%]


void process_image_start(void);
uint16_t * image_analyse(const uint8_t* image, bool is_reading_bottom);
void outlier_detection(uint16_t* lines_position, uint16_t (*lines_pos_history)[MAX_LINE_NBR], uint8_t line_nbr);
void sendnote2buzzer(uint16_t *bottom_pos);
void path_processing(image_bottom, image_top);

#endif /* PROCESS_IMAGE_H */
