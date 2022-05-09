#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define MAX_LINE_NBR 3 // MAX_LINE_NBR - 2 (margins) = nbr of notes permitted at the same time music arrangement (here no arrangement permitted)

void process_image_start(void);

uint16_t * image_analyse(const uint8_t* image, bool is_reading_bottom);
void outlier_detection(uint16_t* lines_position, uint16_t (*lines_pos_history)[MAX_LINE_NBR], uint8_t line_nbr);


#endif /* PROCESS_IMAGE_H */
