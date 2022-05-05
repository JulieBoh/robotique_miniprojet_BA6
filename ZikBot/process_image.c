#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>
#include <leds.h>


#include <process_image.h>

//DEFINE
#define NOISE_RATIO 0.15
#define MIN_LINE_WIDTH 10
#define LINES_POS_HISTORY_SIZE 10
#define SLOPE_WIDTH 5

//static

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);


// CAPTURE IMAGE THREAD //
// The purpose of this thread is to make an acquisition of an image. This image is made by 2 segments: one up and one bottom
static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
    }
}


// PROCESS IMAGE THREAD //
// The purpose of this thread is to analyse the image, to deduce a note and a path and to actuate the motor to track the path.

static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};
	uint16_t* bottom_pos = {0}; //bottom margins and notes position
	//uint16_t top_pos[2] = {0}; //top margins


	bool send_to_computer = true;

	while(1){
		//waits until an image has been captured
		chBSemWait(&image_ready_sem);

		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();
		
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			//Extracts only the green pixels
				// LSBs of high byte
			uint8_t pix_hi = img_buff_ptr[i];
			pix_hi = (0b00000111 & pix_hi)<<3;
				//MSBs of low byte
			uint8_t pix_lo = img_buff_ptr[i+1];
			pix_lo = (0b11100000 & pix_lo)>>5;
				//combine both in image to be sent
			image[i/2] = pix_hi | pix_lo;
			//
		}

		/* TO BE DONE : to be coded in many functions for the readability of the file*/
			// Analyse bottom image -> store bottom margin position
			//						-> identify note
			bottom_pos = bottom_image_analyse(image);

			// Call buzzer giving the note
			// Analyse top image -> store top margin position
			// Calculate path's center and angle
			// Regulator
			// Calculate motorspeed
			// Call motor


		// Send to computer (debug purposes)
		if(send_to_computer){
			//sends to the computer the image
			/*for(int i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
				image[i] = 0;
			}*/
			for(uint8_t i = 0; i<MAX_LINE_NBR; i++){
				if(bottom_pos[i] !=0)
					image[bottom_pos[i]] = 100;
			}
			SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
		}
		//invert the bool
		send_to_computer = !send_to_computer;
		
	}
}


void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}



uint16_t * bottom_image_analyse(uint8_t image[]){
	bool in_line = false;
	uint8_t noise = 0, line_nbr = 0;
	uint8_t comparison_value = image[0];
	uint16_t begin[MAX_LINE_NBR] = {0};
	uint16_t end[MAX_LINE_NBR] = {0};
	static uint16_t lines_position[MAX_LINE_NBR] = {0};
	static uint16_t begin_pos_history[LINES_POS_HISTORY_SIZE][MAX_LINE_NBR] = {0};
	static uint16_t end_pos_history[LINES_POS_HISTORY_SIZE][MAX_LINE_NBR] = {0};
	uint16_t i = 0, width_line = 0;
	uint32_t mean = 0;
		
	//performs an average to now the noise threashold
	for(i = 0 ; i < IMAGE_BUFFER_SIZE -1 ; i++){
		mean += image[i];
	}
	mean /= IMAGE_BUFFER_SIZE;
	noise = mean * NOISE_RATIO;
	
	//analyse buffer with slope detection
	i=0;
	while(i < IMAGE_BUFFER_SIZE-SLOPE_WIDTH){
		//if below the threshold : begin
		if(image[i+SLOPE_WIDTH] < (comparison_value - noise)){
	        width_line++;
			if(width_line == MIN_LINE_WIDTH - 1){ //when the margin reach the desired width
				begin[line_nbr] = i - (width_line - 2);
				in_line = true;
			}
		}
		//if above the threshold : end
		else{
			if(in_line){
				if(line_nbr < MAX_LINE_NBR){
					end[line_nbr] = i;
					line_nbr++;
				}
				in_line = false;
			}
			width_line = 0;
			comparison_value = image[i+1];
		}
		i++;
	}

	//begin and end value filtering
	outlier_detection(begin, begin_pos_history, line_nbr);
	outlier_detection(end, end_pos_history, line_nbr);

	lines_position[0] = end[0];//(end[0]+begin[0])/2;
	lines_position[2] = end[2];//(end[line_nbr-1]+begin[line_nbr-1])/2;
	//Note detected position
	if(line_nbr == 3){
		lines_position[1] = end[1];//(end[1]+begin[1])/2;
	}
	else{
		lines_position[1] = 0;
	}

	return lines_position;
}

void outlier_detection(uint16_t *lines_position, uint16_t (*lines_pos_history)[MAX_LINE_NBR], uint8_t line_nbr){
	uint8_t i = 0;
	uint16_t history_mean = 0;

	//RETURN: table with all positions (margin + notes)
	if(line_nbr == 3 || line_nbr == 2){ //margins (+ note)
		//History and mean update (mean is calculated for note even if it is not necessary)
		for(uint8_t l = 0; l<MAX_LINE_NBR; l++){
			//shift history
			for(uint8_t k = 1; k<LINES_POS_HISTORY_SIZE; k++){
				lines_pos_history[k][l] = lines_pos_history[k-1][l];
			}
			//the actual position is stored into the first position of history
			lines_pos_history[0][l] = lines_position[l];
		}

		//Outlier detection and substitution: not used if we are in the first image acquisition
			//outlier detection margin left
		if((lines_position[0] > lines_pos_history[1][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[1][0]-MIN_LINE_WIDTH) ||
			(lines_position[0] > lines_pos_history[2][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[2][0]-MIN_LINE_WIDTH) ||
			(lines_position[0] > lines_pos_history[3][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[3][0]-MIN_LINE_WIDTH) ||
			(lines_position[0] > lines_pos_history[4][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[4][0]-MIN_LINE_WIDTH) ||
			(lines_position[0] > lines_pos_history[5][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[5][0]-MIN_LINE_WIDTH) ||
			(lines_position[0] > lines_pos_history[6][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[6][0]-MIN_LINE_WIDTH) ||
			(lines_position[0] > lines_pos_history[7][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[7][0]-MIN_LINE_WIDTH) ||
			(lines_position[0] > lines_pos_history[8][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[8][0]-MIN_LINE_WIDTH) ||
			(lines_position[0] > lines_pos_history[9][0]+MIN_LINE_WIDTH) || (lines_position[0] < lines_pos_history[9][0]-MIN_LINE_WIDTH)){
				history_mean = 0;
				i=0;
				for(i=1; i<10; i++){
					history_mean += lines_pos_history[i][0];
				}
				history_mean /= 9;
				lines_position[0] = history_mean;
			}
			//outlier detection margin right
		if((lines_position[2] > lines_pos_history[1][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[1][2]-MIN_LINE_WIDTH) ||
			(lines_position[2] > lines_pos_history[2][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[2][2]-MIN_LINE_WIDTH) ||
			(lines_position[2] > lines_pos_history[3][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[3][2]-MIN_LINE_WIDTH) ||
			(lines_position[2] > lines_pos_history[4][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[4][2]-MIN_LINE_WIDTH) ||
			(lines_position[2] > lines_pos_history[5][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[5][2]-MIN_LINE_WIDTH) ||
			(lines_position[2] > lines_pos_history[6][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[6][2]-MIN_LINE_WIDTH) ||
			(lines_position[2] > lines_pos_history[7][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[7][2]-MIN_LINE_WIDTH) ||
			(lines_position[2] > lines_pos_history[8][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[8][2]-MIN_LINE_WIDTH) ||
			(lines_position[2] > lines_pos_history[9][2]+MIN_LINE_WIDTH) || (lines_position[2] < lines_pos_history[9][2]-MIN_LINE_WIDTH)){
				history_mean = 0;
				i=0;
				for(i=1; i<10; i++){
					history_mean += lines_pos_history[i][2];
				}
				history_mean /= 9;
				lines_position[2] = history_mean;
			}
			//outlier detection note -> is it the same than the 2 previous
		if((lines_position[1] > lines_pos_history[1][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[1][1]-MIN_LINE_WIDTH) ||
			(lines_position[1] > lines_pos_history[2][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[2][1]-MIN_LINE_WIDTH) ||
			(lines_position[1] > lines_pos_history[3][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[3][1]-MIN_LINE_WIDTH) ||
			(lines_position[1] > lines_pos_history[4][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[4][1]-MIN_LINE_WIDTH) ||
			(lines_position[1] > lines_pos_history[5][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[5][1]-MIN_LINE_WIDTH) ||
			(lines_position[1] > lines_pos_history[6][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[6][1]-MIN_LINE_WIDTH) ||
			(lines_position[1] > lines_pos_history[7][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[7][1]-MIN_LINE_WIDTH) ||
			(lines_position[1] > lines_pos_history[8][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[8][1]-MIN_LINE_WIDTH) ||
			(lines_position[1] > lines_pos_history[9][1]+MIN_LINE_WIDTH) || (lines_position[1] < lines_pos_history[4][1]-MIN_LINE_WIDTH)){
				history_mean = 0;
				i=0;
				for(i=1; i<10; i++){
					history_mean += lines_pos_history[i][1];
				}
				history_mean /= 9;
				lines_position[1] = history_mean;
			}
	}
	//Error: if more than MAX_LINE_NBR is detected, nothing is played, and the 2 border-lines are taken as margins.
	// Also if no notes are detected
	else{
		//error
		//TO DO on vie sur l'historique un moment puis on coupe les moteurs et on lance le protocole de fin

		lines_position[0] = 1;//lines_pos_history[1][0];
		lines_position[1] = 1;//lines_pos_history[1][1];
		lines_position[2] = 1;//lines_pos_history[1][2];

		//TO DO: si ça fait trop longtemps juste annule toute sorti -> lines_pos_history = 0
	}
}



