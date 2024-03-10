/*
--------------------------------------------------------------------------
* Acoustic monitoring tool for Raspberry Pi based on miniaudio framework *
*                                                                        *
* Kaue Werner, 2024                                                      *
--------------------------------------------------------------------------
*/
/**
 * @file audio_proc.h
 * @author Kaue Werner
 * @date 10 Mar 2024
 * @brief Header with audio processing functions used in AMT
 * @version 0.1.0
*/
#ifndef AUDIO_PROC
#define AUDIO_PROC
#include "../config_defines.h"

/**
 * @brief Current available types of biquad filter
 *
*/
typedef enum {
	PEQ,
	LPF,
	HPF,
	BPF,
	APF,
	NOTCH,
	HIGHSHELF
} biquad_filter_type;

/**
 * @brief Biquad filter data struct
 *
*/
typedef struct {
    unsigned filterType;
    double cutoffFrequency;
    double qFactor;
    double gain;
    double sampleRate;
    double* coeffs;
    float* previousInput;
    float* previousOutput;
} biquad_filter_data;

/**
 * @brief filter processing function
 * 
*/
void process_filter(float* buffer, unsigned numberOfSamplesToBeProcessed, float * previousInput, float* previousOutput, double *coeffs);

/**
 * @brief compute biquad filter coefficients
 * 
*/
void compute_biquad_filter_coeffs(double* coeffs, unsigned filterType, double fc, double q, double gain, double fs);

/**
 * @brief compute FFT of sample buffer
 * Not being used at the moment, but computation load was tested in the past 
 * and maybe it will used for future implementation... 
*/
void compute_fft(float *input, unsigned bufferSize);

/**
 * @brief compute RMS of sample buffer, with output option set by flagLevel (either amplitude or dB)
 * 
*/
float compute_rms(float* input, unsigned size, int flagLevel);

/**
 * @brief initialize biquad filter (biquad_filter_data)
 * 
*/
void init_filter(biquad_filter_data* filter);

/**
 * @brief free biquad filter (biquad_filter_data)
 * 
*/
void free_filter(biquad_filter_data* filter);

#endif // AUDIO_PROC