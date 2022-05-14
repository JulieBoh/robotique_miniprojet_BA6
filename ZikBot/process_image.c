#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>
#include <leds.h>
#include <math.h>
#include <motors.h>

#include <audio/play_melody.h>
#include "audio/audio_thread.h"

#include <process_image.h>

//DEFINE
#define NOISE_RATIO 0.15
#define MIN_LINE_WIDTH 10
#define LINES_POS_HISTORY_SIZE 10
#define EDGE_WIDTH 5
#define BASE_MOTOR_SPEED 200

//global
static uint8_t note_rel_pos; //[%]
static uint16_t lines_position[MAX_LINE_NBR] = {0};
static uint8_t image[IMAGE_BUFFER_SIZE];


//semaphore
static BSEMAPHORE_DECL(image_captured_sem, TRUE);

//functions
void image_analyse(void);
//void outlier_detection(uint16_t* lines_position, uint16_t (*lines_pos_history)[MAX_LINE_NBR], uint8_t line_nbr);
void sendnote2buzzer(void);
void path_processing(void);
//uint8_t get_rel_pos(void);

// CAPTURE IMAGE BOTTOM THREAD //
// The purpose of this thread is to make an acquisition of an image. This image is made by 2 segments: one up and one bottom
static THD_WORKING_AREA(waCaptureImageBottom, 256);
static THD_FUNCTION(CaptureImageBottom, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 0 + 1 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 300, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_captured_sem);
    }
}
// PROCESS IMAGE THREAD //
// The purpose of this thread is to analyse the image, to deduce a note and a path and to actuate the motor to track the path.

static THD_WORKING_AREA(waProcessImage, 2048);// TEST 1024
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//uint8_t image_resultat[IMAGE_BUFFER_SIZE] = {0};

	uint8_t * image_buffer;
//	uint16_t* pos_ptr; //bottom margins positions [left, right]

//	uint16_t pos[MAX_LINE_NBR]; //bottom margins positions [left, right]

//	bool send_to_computer = true;

	while(1){
		//waits until an image has been captured
		chBSemWait(&image_captured_sem);

		image_buffer = dcmi_get_last_image_ptr();

		for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE*4 ; i+=2){
			//Extracts only the green pixels
				// LSBs of high byte
			uint8_t pix_hi = image_buffer[i];
			pix_hi = (0b00000111 & pix_hi)<<3;
				//MSBs of low byte
			uint8_t pix_lo = image_buffer[i+1];
			pix_lo = (0b11100000 & pix_lo)>>5;
				//combine both in image to be sent
			image[i/2] = pix_hi | pix_lo;
		}

		// Analyse bottom image -> store bottom margin position
		//						-> identify note
		image_analyse();
		// Call buzzer giving the note
		sendnote2buzzer();
		// Calculate path's center and angle
		path_processing();
	}
}


void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImageBottom, sizeof(waCaptureImageBottom), NORMALPRIO, CaptureImageBottom, NULL);
}
void image_analyse(void)
{

	bool in_line = false;
	uint8_t noise_threshold = 0;
	uint8_t line_nbr = 0;
	uint8_t comparison_value = image[0];
	uint16_t line_width = 0;
	uint16_t falling_edges[MAX_LINE_NBR] = {0};
	uint16_t rising_edges[MAX_LINE_NBR] = {0};
	uint32_t mean = 0;

	//performs an average to know the noise threshold
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE; i++)
	{
		mean += image[i];
	}
	mean /= IMAGE_BUFFER_SIZE;
	noise_threshold = mean * NOISE_RATIO;

	/*
	 * for loop holds a comparison value before the falling edge and counts the number of "low" pixels after a falling edge.
	 * once it is certain it is a line, a bool, in_line, is set as true to acknowledge that.
	 * if in_line is true, when a rising edge is detected it is recorded and the comparison value is updated.
	 * if no falling edge is detected, of if the detected line is not wide enough, the comparison value is updated.
	 */
	for(uint16_t i = 0 ; i < (IMAGE_BUFFER_SIZE-EDGE_WIDTH-1); i++)
	{
		if(line_nbr >= MAX_LINE_NBR)
			break;
		chprintf((BaseSequentialStream *)&SD3, "line number = %u \r\n", line_nbr);
		if(image[i+EDGE_WIDTH] < (comparison_value - noise_threshold))
		{ //if we detect a falling edge we
	        line_width++;
			if(line_width == MIN_LINE_WIDTH)
			{ //record falling edge position once line is wide enough
				falling_edges[line_nbr] = i - (line_width - 2);
				in_line = true; //tells us we can look for a rising edge
			}
		}

		else{
			if(in_line){
				if(line_nbr < MAX_LINE_NBR){
					rising_edges[line_nbr] = i;
					line_nbr++;

				}
				in_line = false;
			}
			line_width = 0;
			comparison_value = image[i];
		}
	}
	switch(line_nbr)
	{
	case 2:
		lines_position[0] = (rising_edges[0]+falling_edges[0])/2;
		lines_position[1] = 0;
		lines_position[2] = (rising_edges[1]+falling_edges[1])/2;
		break;
	case 3:
		lines_position[0] = (rising_edges[0]+falling_edges[0])/2;
		lines_position[1] = (rising_edges[1]+falling_edges[1])/2;
		lines_position[2] = (rising_edges[2]+falling_edges[2])/2;
		break;
	default:
		lines_position[0] = 0;
		lines_position[1] = 0;
		lines_position[2] = 0;
	}
}
//void image_analyse(const uint8_t* image){
//	bool in_line = false;
//	uint8_t noise = 0, line_nbr = 0;
//	uint8_t comparison_value = image[0];
//	uint16_t i = 0, width_line = 0;
//	uint16_t begin[MAX_LINE_NBR] = {0};
//	uint16_t end[MAX_LINE_NBR] = {0};
//	uint32_t mean = 0;
//
////	static uint16_t begin_history[LINES_POS_HISTORY_SIZE][MAX_LINE_NBR] = {0};
////	static uint16_t end_history[LINES_POS_HISTORY_SIZE][MAX_LINE_NBR] = {0};
//
//	//performs an average to know the noise threshold
//	for(i = 0 ; i < IMAGE_BUFFER_SIZE -1 ; i++){
//		mean += image[i];
//	}
//
//	mean /= IMAGE_BUFFER_SIZE;
//	noise = mean * NOISE_RATIO;
//
//	//analyse buffer with slope detection
//	i=0;
//	while((i < IMAGE_BUFFER_SIZE-SLOPE_WIDTH) && (line_nbr < MAX_LINE_NBR)){
//		//if below the threshold : begin
//		if(image[i+SLOPE_WIDTH] < (comparison_value - noise)){
//	        width_line++;
//			if(width_line == MIN_LINE_WIDTH - 1){ //when the margin reach the desired width
//				begin[line_nbr] = i - (width_line - 2); //explainn why -2
//				in_line = true;
//			}
//		}
//		//if above the threshold : end
//		else{
//			if(in_line){
//				if(line_nbr < MAX_LINE_NBR){
//					end[line_nbr] = i;
//					line_nbr++;
//				}
//				in_line = false;
//			}
//			width_line = 0;
//			comparison_value = image[i+1];
//		}
//		i++;
//	}
//	//begin and end value filtering
////	outlier_detection(begin, begin_history, line_nbr);
////	outlier_detection(end, end_history, line_nbr);
//
//	lines_position[0] = (end[0]+begin[0])/2;
//	lines_position[2] = (end[line_nbr-1]+begin[line_nbr-1])/2;
//	//Note detected position
//	if(line_nbr == 3){
//		lines_position[1] = (end[1]+begin[1])/2;
//	}
//	else{
//		lines_position[1] = 0;
//	}
//	//return lines_position;
//}
/* TO DO*/
//void outlier_detection(uint16_t *new_lines_pos, uint16_t (*lines_pos_history)[MAX_LINE_NBR], uint8_t line_nbr){
//	uint8_t i = 0;
//	uint16_t history_mean = 0;
//
//	//RETURN: table with all positions (margin + notes)
//	if(line_nbr == 3 || line_nbr == 2){ //margins (+ note)
//		//History and mean update (mean is calculated for note even if it is not necessary)
//		for(uint8_t l = 0; l<MAX_LINE_NBR; l++){
//			//shift history
//			for(uint8_t k = 1; k<LINES_POS_HISTORY_SIZE; k++){
//				lines_pos_history[k][l] = lines_pos_history[k-1][l];
//			}
//			//the actual position is stored into the first position of history
//			lines_pos_history[0][l] = new_lines_pos[l];
//		}
//
//		//Outlier detection and substitution: not used if we are in the first image acquisition
//			//outlier detection margin left
//		if((new_lines_pos[0] > lines_pos_history[1][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[1][0]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[0] > lines_pos_history[2][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[2][0]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[0] > lines_pos_history[3][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[3][0]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[0] > lines_pos_history[4][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[4][0]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[0] > lines_pos_history[5][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[5][0]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[0] > lines_pos_history[6][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[6][0]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[0] > lines_pos_history[7][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[7][0]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[0] > lines_pos_history[8][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[8][0]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[0] > lines_pos_history[9][0]+MIN_LINE_WIDTH) || (new_lines_pos[0] < lines_pos_history[9][0]-MIN_LINE_WIDTH)){
//				history_mean = 0;
//				i=0;
//				for(i=1; i<10; i++){
//					history_mean += lines_pos_history[i][0];
//				}
//				history_mean /= 9;
//				new_lines_pos[0] = history_mean;
//			}
//			//outlier detection margin right
//		if((new_lines_pos[2] > lines_pos_history[1][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[1][2]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[2] > lines_pos_history[2][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[2][2]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[2] > lines_pos_history[3][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[3][2]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[2] > lines_pos_history[4][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[4][2]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[2] > lines_pos_history[5][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[5][2]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[2] > lines_pos_history[6][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[6][2]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[2] > lines_pos_history[7][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[7][2]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[2] > lines_pos_history[8][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[8][2]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[2] > lines_pos_history[9][2]+MIN_LINE_WIDTH) || (new_lines_pos[2] < lines_pos_history[9][2]-MIN_LINE_WIDTH)){
//				history_mean = 0;
//				i=0;
//				for(i=1; i<10; i++){
//					history_mean += lines_pos_history[i][2];
//				}
//				history_mean /= 9;
//				new_lines_pos[2] = history_mean;
//			}
//			//outlier detection note -> is it the same than the 2 previous
//		if((new_lines_pos[1] > lines_pos_history[1][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[1][1]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[1] > lines_pos_history[2][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[2][1]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[1] > lines_pos_history[3][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[3][1]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[1] > lines_pos_history[4][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[4][1]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[1] > lines_pos_history[5][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[5][1]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[1] > lines_pos_history[6][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[6][1]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[1] > lines_pos_history[7][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[7][1]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[1] > lines_pos_history[8][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[8][1]-MIN_LINE_WIDTH) ||
//			(new_lines_pos[1] > lines_pos_history[9][1]+MIN_LINE_WIDTH) || (new_lines_pos[1] < lines_pos_history[4][1]-MIN_LINE_WIDTH)){
//				history_mean = 0;
//				i=0;
//				for(i=1; i<10; i++){
//					history_mean += lines_pos_history[i][1];
//				}
//				history_mean /= 9;
//				new_lines_pos[1] = history_mean;
//			}
//	}
//	//Error: if more than MAX_LINE_NBR is detected, nothing is played, and the 2 border-lines are taken as margins.
//	// Also if no notes are detected
//	else{
//		//error
//		//TO DO: on vit sur l'historique un moment puis on coupe les moteurs et on lance le protocole de fin
//
//		new_lines_pos[0] = 1;//lines_pos_history[1][0];
//		new_lines_pos[1] = 1;//lines_pos_history[1][1];
//		new_lines_pos[2] = 1;//lines_pos_history[1][2];
//
//		//TO DO: si ça fait trop longtemps juste annule toute sorti -> lines_pos_history = 0
//	}
//}
#define SCALE_SIZE 5
#define REL_POS_THRESHOLD (100/SCALE_SIZE)
//static const uint16_t c_major_scale[SCALE_SIZE] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
//static const uint16_t c_major_scale[SCALE_SIZE] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_B4, NOTE_D4};

void sendnote2buzzer(void){

	dac_stop();
	uint16_t c_major_scale[SCALE_SIZE] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_B4, NOTE_D4};
	//if there's no note skip
	if(lines_position[1] == 0)
		return;
	//else
	//compute note relative position
	note_rel_pos = (lines_position[1]-lines_position[0])*100/(lines_position[2]-lines_position[0]); //in % to avoid a float
    for(uint8_t i=0; i < SCALE_SIZE; i++)
    {
    	if(note_rel_pos < (i+1)*REL_POS_THRESHOLD)
    	{
    		uint16_t note = c_major_scale[i];
    		dac_play(note);
    	}
    }
	//envoie note + temps
}

//uint8_t get_rel_pos(void)
//{
//	return note_rel_pos;
//}


void path_processing(void){
	if((lines_position[0] == 0) && (lines_position[2] == 0))
		return;
	static int16_t motor_speed = 0;
	int16_t robot_angle = ((lines_position[0]+lines_position[2])/2.) - (IMAGE_BUFFER_SIZE/2);
	
	//filter
	if(abs(robot_angle)>200)
		robot_angle = 0;
	motor_speed = (3*robot_angle/5) + 2*motor_speed/5;

	//motor
	right_motor_set_speed(BASE_MOTOR_SPEED-motor_speed);
	left_motor_set_speed(BASE_MOTOR_SPEED+motor_speed);

	//return robot_angle;
}

