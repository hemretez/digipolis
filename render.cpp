/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
  Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
  Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/


#include <Bela.h>
#include <Midi.h>
#include <stdlib.h>
#include <rtdk.h>
#include <cmath>
#include "variables.h"

float gFreq;
float gPhaseIncrement = 0;
bool gIsNoteOn = 0;
int gVelocity = 0;
float gSamplingPeriod = 0;

// my 
int gMIDI_Notes[NUMBER_OF_STAGES] = {36,42,36,46,36,36,36,50,36,36,36,36,36,36,36,36};
float gFrequencies[NUMBER_OF_STAGES];
int gCurrentStage = 0;
int gGateState;
int gGateModes[NUMBER_OF_STAGES] = {0,1,0,2,0,0,0,1,0,0,0,0,0,0,0,0,};

int gReadPointers[NUMBER_OF_STAGES] = {0};
int gDrumBufferForReadPointer[NUMBER_OF_STAGES] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; 

int ledPin = P8_09;			// digital pin 2 for LED
float potRead = 0;			// variable for the potentiometer reading
float pot2Read = 0;			// variable for the potentiometer reading


int gAudioFramesPerAnalogFrame;	// Audio frames per analog frame for analog
int gAudioFrameRate;

// Set the analog channels to read from
int gAnalogSensorInputAmplitude = 1;

int gEventIntervalMilliseconds = 250;
int gEventIntervalInSamples;
int gGateTimeInMilliseconds = 50;
int gGateTimeInSamples;
long gNumSamplesPassed;
int gCount = 0;

float gInterval = 0.02; // Fraction of the second to read the accelerometer and execute the patterns: 50ms = 20 times a second

//Synth
float gEnvelopeDecayRate = 0.9996;    // Decay rate: 0.9996 closer to 1 = longer decay
float gEnvelopeAmplitude = 0;     // Current value of the envelope
float gPhase;
float gInverseSampleRate;

/*
 * This callback is called every time a new input Midi message is available
 *
 * Note that this is called in a different thread than the audio processing one.
 *
 */
void midiMessageCallback(MidiChannelMessage message, void* arg){
	if(arg != NULL){
		rt_printf("Message from midi port %s ", (const char*) arg);
	}
	message.prettyPrint();
	if(message.getType() == kmmNoteOn){
		gFreq = powf(2, (message.getDataByte(0)-69)/12.0f) * 440;
		
		gVelocity = message.getDataByte(1);
		gPhaseIncrement = 2 * M_PI * gFreq * gSamplingPeriod;
		gIsNoteOn = gVelocity > 0;
		rt_printf("v0:%f, ph: %6.5f, gVelocity: %d, midiData1: %d\n", gFreq, gPhaseIncrement, gVelocity, message.getDataByte(0) );
	}
}

Midi midi;

const char* gMidiPort0 = "hw:1,0,0";

bool setup(BelaContext *context, void *userData)
{
	midi.readFrom(gMidiPort0);
	midi.writeTo(gMidiPort0);
	midi.enableParser(true);
	midi.setParserCallback(midiMessageCallback, (void*) gMidiPort0);
	
	if(context->analogFrames == 0) {
		rt_printf("Error: this example needs the analog I/O to be enabled\n");
		return false;
	}

	if(context->audioOutChannels < 2 ||
		context->analogOutChannels < 2){
		printf("Error: for this project, you need at least 2 analog and audio output channels.\n");
		return false;
	}
	
	/* Initialise GPIO pins */
	pinMode(context, 0, ledPin, OUTPUT);
	

	// Useful calculations
	gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
	// Tempo = 60 * 1000 / gEventIntervalMilliseconds;
	gEventIntervalInSamples = int(context->audioSampleRate / (1000 / gEventIntervalMilliseconds));
	gGateTimeInSamples = int(context->audioSampleRate / (1000 / gGateTimeInMilliseconds));
	
	gAudioFrameRate = int(context->audioSampleRate);	
	gInverseSampleRate = 1.0/context->audioSampleRate;
  	gPhase = 0.0;
	gSamplingPeriod = 1/context->audioSampleRate;
	
	return true;
}

// render() is called regularly at the highest priority by the audio engine.
// Input and output are given from the audio hardware and the other
// ADCs and DACs (if available). If only audio is available, numMatrixFrames
// will be 0.

enum {kVelocity, kNoteOn, kNoteNumber};

void render(BelaContext *context, void *userData){
	
	for(unsigned int n = 0; n < context->audioFrames; n++){
		
		// On even audio samples: read analog inputs and update
		if(!(n % gAudioFramesPerAnalogFrame)) {
			potRead = (constrain(map(analogRead(context, n/gAudioFramesPerAnalogFrame, 0), 0, 0.8, 50, 2000), 50, 2000));
			pot2Read = (constrain(map(analogRead(context, n/gAudioFramesPerAnalogFrame, 1), 0, 0.8, 1, potRead), 1, potRead));
			
			//gTempo = 60 * 1000 / potRead;
			gEventIntervalInSamples = int( context->audioSampleRate / (1000.0 / potRead) );
			gGateTimeInSamples = int( context->audioSampleRate / (1000.0 / pot2Read) );
			
		  	//rt_printf("%f \n",potRead);

		}		
		gCount++;
		// Reding of the MIDI commands and assigning to the variables are not made every audio or analog frame
		// It is determined by the gInterval. For example, if gInterval is 0.1, then 10 times a second
		if(gCount % (int)(context->audioSampleRate * gInterval) == 0) {
			
			
		}


		// This bit is where the tempo is set
		// Potentiometer reading changes gEventIntervalInSamples 
		gNumSamplesPassed++; 
		// If the number of samples passed for the current tempo is enough 
		// and gIsPlaying is set to 1 (playing state), then start the next event
		if(gNumSamplesPassed >= gEventIntervalInSamples ) {	
			//if(gIsPlaying == 1 )
			//	startNextEvent();
			
			gNumSamplesPassed = 0; // set gNumSamplesPassed to 0 again to start counting from 0 again for the next event
		}		
		
    
    // Calculate the sample, using the current amplitude from the envelope
	int frequency = 440;
    
    float out = gEnvelopeAmplitude * sinf(gPhase);
    gPhase += 2.0 * M_PI * frequency * gInverseSampleRate;
    if(gPhase > 2.0 * M_PI)
      gPhase -= 2.0 * M_PI;

    // make the envelope decay over time
    gEnvelopeAmplitude *= gEnvelopeDecayRate;
 
    for(unsigned int channel = 0; channel < context->audioOutChannels; channel++){
    	context->audioOut[n * context->audioOutChannels + channel] = out;
    }
		
	}
}

// cleanup() is called once at the end, after the audio has stopped.
// Release any resources that were allocated in setup().

void cleanup(BelaContext *context, void *userData)
{

}

/**
\example 05-Communication/MIDI/render.cpp

Connecting MIDI devices to Bela!
-------------------------------

This example needs documentation.

*/
