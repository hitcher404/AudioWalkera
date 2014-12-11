#include <iostream>

#include "audiowalkera.h"

#include <thread>

using namespace std;

int main(){

	cout << "Walkera PCM" << endl;

	AudioWalkera aw;

	aw.init();

	aw.enableJoystick();

	thread t = aw.startThread();


	array<int, 5> values;

	for(;;){

		aw.getValues(values);

		cout << "\r";
		for(int i = 0; i < 5; i++){
		   	cout << "    " << values[i];
		}
		cout << "        " << flush;

		this_thread::sleep_for(chrono::milliseconds(100));
	}


	//t.join();


}
