#include <main.h>
#include <camera/po8030.h>

#include <capture_and_play.h>

static BSEMAPHORE_DECL(capture_and_play_sem, TRUE);

static THD_WORKING_AREA(waCaptureAndPlay, 256);
static THD_FUNCTION(CaptureAndPlay, arg) {
	// Thread Init
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    //From TP4 for camera setup
	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

	// Body
    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&capture_and_play_sem);

		// camera -> capture ROI
		// store ROI in array
		// center new ROI
		// translate ROI in note
		// play note
		// stop thread for a time fiven by the tempo
    }
}

void capture_and_play_start(void){
	chThdCreateStatic(waCaptureAndPlay, sizeof(waCaptureAndPlay), NORMALPRIO, CaptureAndPlay, NULL);
}
