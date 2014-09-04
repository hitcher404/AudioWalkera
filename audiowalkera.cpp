#include <alsa/asoundlib.h>

#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>



#include <iostream>


bool debug = true;


int16_t rudder, elevator,ailerons,throttle,flipSwitch;



int setup_hardware(snd_pcm_t *handle, snd_pcm_hw_params_t *params, int * dir){
	snd_pcm_uframes_t frames;
	unsigned int val;

	snd_pcm_hw_params_any(handle, params);

	snd_pcm_hw_params_set_access(handle, params, 
			SND_PCM_ACCESS_RW_INTERLEAVED);

	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

	snd_pcm_hw_params_set_channels(handle, params, 1);



	val = 48000;
	snd_pcm_hw_params_set_rate_near(handle, params, &val, dir);

	frames = 2304;
	snd_pcm_hw_params_set_period_size_near(handle, params, &frames, dir);

	return snd_pcm_hw_params(handle, params);
}


int processBin(int val){
	int ret = 0;
	if(val < 17){
		ret = 0;
	}else{
		ret = 1;
	}
	return ret;
}

int processOct(int val){

	int ret = 0;

	if(val < 17){
		ret = 0;
	}else if(val < 21){
		ret = 1;
	}else if (val < 25){
		ret = 2;
	}else if (val < 29){
		ret = 3;
	}else if (val < 33){
		ret = 4;
	}else if (val < 36){
		ret = 5;
	}else if (val < 40){
		ret = 6;
	}else{
		ret = 7;
	}

	return ret;

}


void processAll( int* processed){



	int msb;
	msb = processed[16]&1;

	rudder = msb*256+processed[17]*128+processed[18]*16+processed[19]*8+processed[20];

	elevator = processed[2]*64+processed[3]*32+processed[4]*4+processed[5]*2+(processed[6]>>2);

	msb = processed[6]&1;

	ailerons = msb*256+processed[7]*128+processed[8]*16+processed[9]*8+processed[10];

	throttle = processed[12]*64+processed[13]*32+processed[14]*4+processed[15]*2+((processed[16]&4)>>2);

	flipSwitch = processed[23];


	if(!processed[11]) throttle = -throttle;


	if(!(processed[6]&2)) ailerons = -ailerons;


	if(!processed[1]) elevator = -elevator;


	if(!(processed[16]&2)) rudder = -rudder;


	if(debug){
		std::cout << throttle << "  " << elevator << "  " << 
			rudder << "  " << ailerons << "  " << flipSwitch << "  " << std::endl;
	}


}

void exitError(const char* err){

	std::cout << err << std::endl;

	exit(1);

}

int main(int argc, char *argv[]){




	////////////////////////////////////////////UINPUT
	int                    fd;
	struct uinput_user_dev uidev;
	struct input_event     ev;
	double                    dx, dy;
	int                    k;

	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(fd < 0)
		exitError("error: open");

	if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
		exitError("error: ioctl");
	if(ioctl(fd, UI_SET_KEYBIT, BTN_JOYSTICK) < 0)
		exitError("error: ioctl");
	if(ioctl(fd, UI_SET_EVBIT, EV_ABS) < 0)
		exitError("error: ioctl");
	if(ioctl(fd, UI_SET_ABSBIT, ABS_X) < 0)
		exitError("error: ioctl");
	if(ioctl(fd, UI_SET_ABSBIT, ABS_Y) < 0)
		exitError("error: ioctl");
	if(ioctl(fd, UI_SET_ABSBIT, ABS_THROTTLE) < 0)
		exitError("error: ioctl");
	if(ioctl(fd, UI_SET_ABSBIT, ABS_RUDDER) < 0)
		exitError("error: ioctl");


	if(ioctl(fd, UI_SET_EVBIT, EV_MSC) < 0)
		exitError("error: ioctl");

	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Walkera-PCM-0701");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1;
	uidev.id.product = 0x1;
	uidev.id.version = 1;

	//Hardcoded calibration values, set these yourself.
	uidev.absmax[ABS_X] = 365;
	uidev.absmin[ABS_X] = -365;

	uidev.absmax[ABS_Y] = 425;
	uidev.absmin[ABS_Y] = -425;

	uidev.absmax[ABS_RUDDER] = 370;
	uidev.absmin[ABS_RUDDER] = -370;

	uidev.absmax[ABS_THROTTLE] = 370;
	uidev.absmin[ABS_THROTTLE] = -370;


	if(write(fd, &uidev, sizeof(uidev)) < 0){
		exitError("Cant setup uinput device, permissions? Udev rule?");
	}
	if(ioctl(fd, UI_DEV_CREATE) < 0){
		exitError("error: ioctl");
	}

	sleep(2);

	srand(time(NULL));
	/////////////////////////////////////////////////////////






	int rc, dir;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
	short *buffer;
	int t = 0, i, c;

	if(argc < 2)
		rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
	else rc = snd_pcm_open(&handle, argv[1], SND_PCM_STREAM_CAPTURE, 0);

	if(rc < 0)
		exitError("Could not open pcm device");

	snd_pcm_hw_params_alloca(&params);
	rc = setup_hardware(handle, params, &dir);

	if(rc < 0)
		exitError("Could not set hardware parameters");

	snd_pcm_hw_params_get_period_size(params, &frames, &dir);

	buffer = (short*) calloc(frames, sizeof(short));


	bool loop = true;


	bool inFrame = false;

	int startIndex = 0;

	int lowCount = 0, highCount = 0, blipCount = 0;

	bool falling = true;

	int values[51];
	int valuesIndex = 0;



	while(loop){


		inFrame = false;
		highCount = 0;
		lowCount = 0;


		rc = snd_pcm_readi(handle, buffer, frames);

		if( rc == -EPIPE )
			fprintf(stderr, "buffer overrun\n");
		else if( rc < 0 )
			fprintf(stderr, "read error: %s\n", snd_strerror(rc));
		else {
			for(i=0; i < rc; i++){
				t++;
				//std::cout << buffer[i] << std::endl;


				if(buffer[i] > 0){
					highCount++;
				}else{
					highCount = 0;
				}

				//if(highCount == 62 && (buffer[i+1] < 0) || highCount == 63){
				if(highCount >= 62 && (buffer[i+1] < 0)){
					//std::cout << "FRAME" << std::endl;
					highCount = 0;
					inFrame = true;
					startIndex = i+1;

					//std::cout << time.elapsed()*1000 << std::endl;
					//time.restart();

					break;
				}


				//usleep(10);
			}

			valuesIndex = 1;
			falling = true;

			while(inFrame){


				if(falling){

					if(buffer[startIndex] < 0){
						lowCount++;
						startIndex++;
					}else{
						values[valuesIndex] = processBin(lowCount);
						lowCount = 0;
						falling = false;
						valuesIndex++;


					}

				}else{
					if(buffer[startIndex] > 0){
						highCount++;
						startIndex++;
					}else{
						values[valuesIndex] = processOct(highCount);
						highCount = 0;
						falling = true;
						valuesIndex++;

					}
				}

				if(valuesIndex == 24) inFrame = false;



			}

			processAll(values);

			dx = rudder;
			dy = throttle;


			memset(&ev, 0, sizeof(struct input_event));
			ev.type = EV_ABS;
			ev.code = ABS_X;
			ev.value = dx;
			if(write(fd, &ev, sizeof(struct input_event)) < 0){
				exitError("error: write event");
			}


			memset(&ev, 0, sizeof(struct input_event));
			ev.type = EV_ABS;
			ev.code = ABS_Y;
			ev.value = dy;
			if(write(fd, &ev, sizeof(struct input_event)) < 0){
				exitError("error: write event");
			}


			memset(&ev, 0, sizeof(struct input_event));
			ev.type = EV_ABS;
			ev.code = ABS_RUDDER;
			ev.value = ailerons;
			if(write(fd, &ev, sizeof(struct input_event)) < 0){
				exitError("error: write event");
			}


			memset(&ev, 0, sizeof(struct input_event));
			ev.type = EV_ABS;
			ev.code = ABS_THROTTLE;
			ev.value = elevator;
			if(write(fd, &ev, sizeof(struct input_event)) < 0){
				exitError("error: write event");
			}


			memset(&ev, 0, sizeof(struct input_event));
			ev.type = EV_KEY;
			ev.code = BTN_JOYSTICK;
			ev.value = flipSwitch;
			if(write(fd, &ev, sizeof(struct input_event)) < 0){
				exitError("error: write event");
			}


			memset(&ev, 0, sizeof(struct input_event));
			ev.type = EV_SYN;
			ev.code = 0;
			ev.value = 0;
			if(write(fd, &ev, sizeof(struct input_event)) < 0){
				exitError("error: write event");
			}
		}
	}


	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	free(buffer);

	return 0;
}
