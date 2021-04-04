/*
 * adsr.h
 *
 *  Created on: Apr 1, 2021
 *      Author: cooper
 */

#ifndef INC_ADSR_H_
#define INC_ADSR_H_

#include <stdint.h>

#ifdef TESTING_ADSR
#include <stdio.h>
#endif
// This file contains logic for the creation of attack, decay, sustain, and release parameters.
// We do not support multiple adsr values for different notes, instead using set of parameters 
// for all notes.

// in kHz
#define SAMPLING_FREQUENCY 48

#ifndef BUFSIZE
#define BUFSIZE 256
#endif

#ifndef MAX_RELEASE
#define MAX_RELEASE 10
#endif

#define MIN_ATTACK 1
#define MIN_DECAY 1
#define MIN_RELEASE 1

#define MAX_ATTACK 0xff
#define MAX_DECAY 0xff
#define MAX_SUSTAIN 0xff
#define MAX_RELEASE 0xff

// in seconds, no smaller than 1
#define ATTACK_MAX_TIME 1
#define DECAY_MAX_TIME 1
#define RELEASE_MAX_TIME 1

#if ATTACK_MAX_TIME == DECAY_MAX_TIME
#if ATTACK_MAX_TIME == RELEASE_MAX_TIME
#define COALESCED 1
#else
#define NOT_COALESCED 1
#endif
#else
#define NOT_COALESCED 1
#endif

#define ATTACK_COUNT_MAX (SAMPLING_FREQUENCY * 1000 * ATTACK_MAX_TIME)
#define DECAY_COUNT_MAX (SAMPLING_FREQUENCY * 1000 * DECAY_MAX_TIME)

#define NOTE_MAX 87
#define NOTE_MIN 0

enum ADSR_STATUS {
	ATTACK,
	DECAY,
	SUSTAIN,
	RELEASE,
	OFF
};

#ifdef TESTING_ADSR
void p_mult(FILE * fp);
#endif

void initializeParameters(uint8_t attack, uint8_t decay, uint8_t sustain, uint8_t release);

float get_multiplier(uint8_t note);

void note_released(uint8_t note);

void note_pressed(uint8_t note);

void sample_finished();
#endif /* INC_ADSR_H_ */
