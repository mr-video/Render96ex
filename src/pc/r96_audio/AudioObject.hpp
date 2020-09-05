#include "audiofile.h"

struct AudioBuffer {
	int bufferSize;
	double* bufferData;
};

struct AudioObject {
		
	AudioFile<double> data;
	AudioBuffer buffer;
	bool loop;

};
