/*
* This header is used to define the most important variables, scales and functions. 
 */

#ifndef _VARIABLES_H
#define _VARIABLES_H

#define NUMBER_OF_POSSIBLE_STAGES 16
#define NUMBER_OF_NOTES 7
#define NUMBER_OF_STAGES 8
#define NUMBER_OF_PULSES 8

#define PI 3.14159265

float lineSegment(int sampleNumber, int totalSamples, float startValue,
				  float endValue);

void sequencerStateMachine_ToCloseGates();
void sequencerStateMachine_ToOpenGates();
void synthe();
void assignMIDIControlInputs(int ctrl, int ctrl_Val);	// function to select patterns by using the state machine
void assignMIDINoteInputs(int midiNote, int midi_Val);	// function to select patterns by using the state machine
void applyScales(int stage);

// SCALES
// chromatic -> 0
int major[7] = {0,2,4,5,7,9,11};			// 1
int diatonicMinor[7] = {0,2,3,5,7,8,10};	// 2
int dorian[7] = {0,2,3,5,7,9,10};			// 3
int locrian[7] = {0,1,3,5,6,8,10};			// 4
int aeolian[7] = {0,2,3,5,7,8,10};			// 5
int turkish[7] = {0,1,3,5,7,10,11};			// 6
int indian[7] = {0,1,1,4,5,8,10};			// 7

#endif /* _VARIABLES_H */
