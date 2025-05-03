/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

http://bela.io

NLMS Adaptive Filter for Adaptive Noise Cancellation
*/
#include <Bela.h>
#include "Debouncer.h"
#include "MonoFilePlayer.h"
#include "NLMS.h"


// set pins
const int kFilterButtonPin = 0;
const int kResetButtonPin = 1;
const int kFilterLED = 2;
const int kResetLED = 3;

int gFilterLEDState = 0;
int gOffState = 1;

bool gFilterOn = false;

// set digital per audio ratio
float gDigitalPerAnalog = 0.0;

// writer pointer is handled in class

// Name of the sound files (in project folder)
std::string gSpeechFile = "noisy_speech2.wav"; 
std::string gNoiseFile = "noisy_3.wav"; 

// Objecst that handle playing sound from a buffer
MonoFilePlayer gSpeechPlayer;
MonoFilePlayer gNoisePlayer;

// debouncer setup
Debouncer gFilterDebouncer;
Debouncer gResetDebouncer;

// NLMS
NLMS gNLMS;

bool setup(BelaContext *context, void *userData)
{
	// setup file players
	if(!gSpeechPlayer.setup(gSpeechFile)) {
    	rt_printf("Error loading audio file '%s'\n", gSpeechFile.c_str());
    	return false;
	}
	
	// gSpeechPlayer.setLoop(true);
	
	if(!gNoisePlayer.setup(gNoiseFile)) {
    	rt_printf("Error loading audio file '%s'\n", gNoiseFile.c_str());
    	return false;
	}
	
	// gNoisePlayer.setLoop(true);
	
	//NLMS filter
	gNLMS.setup(32, 0.0001f, 0.000001);

	// Print some useful info
    rt_printf("Loaded the speech file '%s' with %d frames (%.1f seconds)\n", 
    			gSpeechFile.c_str(), gSpeechPlayer.size(),
    			gSpeechPlayer.size() / context->audioSampleRate);
	rt_printf("Loaded the noise file '%s' with %d frames (%.1f seconds)\n", 
    			gNoiseFile.c_str(), gNoisePlayer.size(),
    			gNoisePlayer.size() / context->audioSampleRate);
    			
    // Initialise the debouncer with 50ms interval
	gFilterDebouncer.setup(context->audioSampleRate, .05);
	gResetDebouncer.setup(context->audioSampleRate, .05);
	
	// I/O setup
	// checking Digital Frames
	if (context->digitalFrames != context->audioFrames){
		rt_fprintf(stderr, "Digital and Audio frames need to be the same.");
		return false;
	}
	
	// Checking Digital is 2x Analog
	if (context->digitalFrames != 2 * context->analogFrames){
		rt_fprintf(stderr, "Digital Frames need to be 2X the Analog frames");
		return false;
	}
	
	// setting Digital per analog frame rate
	if (context->analogFrames){
		gDigitalPerAnalog = context->digitalFrames / context->analogFrames;
	}
	
	// setting pins
	pinMode(context, 0, kFilterButtonPin, INPUT);
	pinMode(context, 0, kResetButtonPin, INPUT);
	pinMode(context, 0, kFilterLED, OUTPUT);
	pinMode(context, 0, kResetLED, OUTPUT);
	
	return true;
}

void render(BelaContext *context, void *userData)
{

    for(unsigned int n = 0; n < context->audioFrames; n++) {
    	
    	float speech_in = gSpeechPlayer.process();
		float noise_in = gNoisePlayer.process();
		
		// filter error output
		float error_out = 0.0;

    	// Read in both button values
    	int filterButtonValue = digitalRead(context, n, kFilterButtonPin);
    	int resetButtonValue = digitalRead(context, n, kResetButtonPin);
    	
    	// The process() method returns whether the button is high
    	// or low right now, but we are interested in the edges:
    	// falling edge is a press, rising edge is a release
    	gFilterDebouncer.process(filterButtonValue);
    	gResetDebouncer.process(resetButtonValue);

		// TURNING FILTER ON OR OFF
    	if(gFilterDebouncer.fallingEdge()) {
    		gFilterOn = true;
    		gFilterLEDState = HIGH;
    		gOffState = LOW;
    	}    	
    	if(gFilterDebouncer.risingEdge()) {
    		gNLMS.reset();
			gFilterOn = false;
			gFilterLEDState = LOW;
			gOffState = HIGH;
    	}
    	
    	if (gFilterOn == true){
    		error_out = gNLMS.step(noise_in, speech_in);
    		// speech_in = y_out;
    	}
    	else if (gFilterOn == false){
    	   	error_out = 0.0;
    	}
    	// AUDIO RESET
    	if(gResetDebouncer.fallingEdge()) {

    	}    	
    	if(gResetDebouncer.risingEdge()) {

    	}
    	
    	float speech_output = speech_in - error_out;
    	
    	
    	// Write the audio to the output
		for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
			audioWrite(context, n, channel, speech_output);
		}
		
		// LEDS
   		digitalWriteOnce(context, n, kFilterLED, gFilterLEDState);
   		digitalWriteOnce(context, n, kResetLED, gOffState);
    	
    }

}

void cleanup(BelaContext *context, void *userData)
{

}