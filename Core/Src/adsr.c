/*
 * adsr.c
 *
 *  Created on: Apr 1, 2021
 *      Author: cooper
 */

#include "adsr.h"

// time to reach max amp
static uint8_t attack = 1;
// time to reach sustain
static uint8_t decay = 1;
// sustain level is the multiplier to its amplitude
// sustain mult = sustain/255
static uint8_t sustain = 0xff;
// release is being ignored rn.
static uint8_t release = 1;

static long attack_sub_counter = 0;
static long decay_sub_counter = 0;

static uint8_t attack_add = 0;
static uint8_t decay_add = 0;

static long attack_time_scale = 1;
static long decay_time_scale = 1;
static long release_time_scale = 1;

static float multiplier_table[MAX_ATTACK + 1 + MAX_DECAY + 1 + MAX_RELEASE];
static long notes_step[NOTE_MAX - NOTE_MIN + 1] = {0};
static enum ADSR_STATUS notes_status[NOTE_MAX - NOTE_MIN + 1] = {OFF};

static uint8_t released_notes[MAX_RELEASE] = {0};
static uint8_t released_length = 0;

#ifdef TESTING_ADSR
void p_mult(FILE * fp) {
	fprintf(fp, "%d, %d, %d\n", attack, decay, sustain);
	for(int i = 0; i < sizeof(multiplier_table)/sizeof(float) - 1; i++) {
		fprintf(fp, "%f, ", multiplier_table[i]);
	}

	fprintf(fp, "%f\n", multiplier_table[sizeof(multiplier_table)/sizeof(float)]);
}
#endif

void initializeParameters(uint8_t l_attack, uint8_t l_decay, uint8_t l_sustain, uint8_t l_release) {
	if(l_attack >= MIN_ATTACK) attack = l_attack;
	if(l_decay >= MIN_DECAY) decay = l_decay;
	if(l_release >= MIN_RELEASE) release = l_release;
	sustain = l_sustain;

	int i = 0;
	for(; i <= attack; i++) {
		multiplier_table[i] = (float)i/(float)attack;
	}

	float fp_sustain = (float)sustain / (float)MAX_SUSTAIN;
	int decay_time = 1;
	for(; i <= attack + decay; i++) {
		multiplier_table[i] = 1+ (fp_sustain - 1.0) * (decay_time++)/decay;
	}

	int release_time = 1;
	for(; i < sizeof(multiplier_table)/ sizeof(float); i++) {
		float newval = fp_sustain * (1 - release_time / release);
		release_time++;
	}
	
	attack_time_scale = (SAMPLING_FREQUENCY * 1000) * ATTACK_MAX_TIME * attack / MAX_ATTACK;
	decay_time_scale = (SAMPLING_FREQUENCY * 1000 ) * DECAY_MAX_TIME * decay / MAX_DECAY;
	release_time_scale = (SAMPLING_FREQUENCY * 1000) * RELEASE_MAX_TIME * release / MAX_RELEASE;
}

void add_release(uint8_t note) {
	released_note[released_length] = note;
	notes_step[note] = MAX_ATTACK+1 + MAX_DECAY+1;
	notes_status[note] = RELEASE;
}

void note_released(uint8_t note) {
	notes_step[note] = 0;
	if(released_length >= MAX_RELEASE) {
		// need to kick out the oldest note
	}
	add_release(note);
}

void note_pressed(uint8_t note) {
	notes_step[note] = 0;
	notes_status[note] = ATTACK;
}

void sample_finished() {
	attack_sub_counter += 1;
	decay_sub_counter += 1;

	// max_release_time = max_attack_time = max_decay_time
	attack_add = attack * attack_sub_counter / attack_time_scale;
	if(attack_add) attack_sub_counter = 0;

#if NOT_COALESCED == 1
	decay_add = decay * decay_sub_counter / decay_time_scale;
	decay_sub_counter = 0;
#endif
}

void note_finished(uint8_t note) {
	notes_status[note] = OFF;
	notes_step[note] = 0;

	// remove from release notes list
}

float get_multiplier(uint8_t note) {
	long time = notes_step[note];
	float mult = multiplier_table[time];
	
#if COALESCED == 1
	if(notes_status[note] == SUSTAIN) return mult;
	time += attack_add;
	if(time > MAX_ATTACK + MAX_DECAY + MAX_RELEASE + 2) {
		note_finished(note);
		return 0.0;
	}
	notes_step[note] = time;
	if(time = attack+decay) notes_status[note] = SUSTAIN;
#else
	if(notes_status[note] == ATTACK) {
		time += attack_add;
		if(time > attack) notes_status[note] = DECAY;
	} else if(notes_status[note] == DECAY) {
		time += decay_add;
		if(time > attack + decay) notes_status[note] = SUSTAIN;
	}
	if(time > MAX_ATTACK + MAX_DECAY + MAX_RELEASE + 2) time = MAX_ATTACK + MAX_DECAY + MAX_RELEASE + 2;
	// we do not increment or anything if at sustain.
	notes_step[note] = time;
#endif


	return mult;
}
