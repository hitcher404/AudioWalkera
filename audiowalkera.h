#ifndef _AUDIOWALKERA_H_
#define _AUDIOWALKERA_H_
#include <alsa/asoundlib.h>


#include <linux/uinput.h>

#include <thread>
#include <mutex>

class AudioWalkera{

	public:
		AudioWalkera();
		void init();
		void enableJoystick();

		std::thread startThread();

		void printValues();
		bool checkNew();

		void getValues(std::array<int, 5>& values);

	private:
		int processBin(int val);
		int processOct(int val);
		void processAll(int* processed);

		void setupAlsa();

		void setupUinput();

		void uinputEvents();
		
		//int setupHardware(snd_pcm_t *handle, snd_pcm_hw_params_t *params, int * dir);

		void exitError(const char* err);

		void readThread();




		int16_t rudder, elevator,ailerons,throttle,flipSwitch;

		bool inFrame;
		int startIndex;
		int lowCount,highCount;


		bool falling = true;

		int values[51];
		int valuesIndex = 0;


		//Alsa structs/variables
		int rc;
		snd_pcm_t *handle;
		snd_pcm_hw_params_t *params;
		snd_pcm_uframes_t frames;
		short *buffer;


		//Uinput structs/variables
		int fd;
		struct input_event ev;
		struct input_event multiEv[4];
		bool uinputEnabled = false;



		bool debug = false;


		bool newData = false;


		std::mutex mutex;
		


};
#endif
