/*
--------------------------------------------------------------------------
* Acoustic monitoring tool for Raspberry Pi based on miniaudio framework *
*                                                                        *
* Kaue Werner, 2024                                                      *
--------------------------------------------------------------------------
*/
/**
 * @file audio_proc.c
 * @author Kaue Werner
 * @date 10 Mar 2024
 * @brief Definition of audio processing functions used in AMT
 * @version 0.1.0
*/
#include "audio_proc.h"
#include <fftw3.h>
#include <stdlib.h>
#include <math.h>

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif

/**
 * @brief filter processing function
 * 
*/
void process_filter(float* buffer, unsigned numberOfSamplesToBeProcessed, float * previousInput, float* previousOutput, double *coeffs){
	for (unsigned n = 0; n < numberOfSamplesToBeProcessed; n++){
		float tmpInput = buffer[n];
		buffer[n] = ((float) coeffs[2] * tmpInput) +
						((float) coeffs[1] * previousInput[0]) +
						((float) coeffs[0] * previousInput[1]) -
						((float) coeffs[4] * previousOutput[0]) -
						((float) coeffs[3] * previousOutput[1]);
		previousInput[1] = previousInput[0];
		previousInput[0] = tmpInput;
		previousOutput[1] = previousOutput[0];
		previousOutput[0] = buffer[n];
    }
}

/**
 * @brief compute biquad filter coefficients
 * 
*/
void compute_biquad_filter_coeffs(double* coeffs, unsigned filterType, double fc, double q, double gain, double fs){
    double w0 = 2*PI*(fc/fs);
    double alpha, a, a0;
    /*__________________ coeffs = [b2, b1, b0, a2, a1] __________________*/ 

    double sinw = sin(w0);
    double cosw = cos(w0);
    a = pow(10, gain/40);
    alpha = 0.5*sinw/q;

    switch(filterType){
        // Peak filter
        case PEQ:
        {
            coeffs[0] = 1 - (alpha*a);  // b2
            coeffs[1] = -2*cosw;        // b1
            coeffs[2] = 1 + (alpha*a);  // b0
            coeffs[3] = 1 - (alpha/a);  // a2
            coeffs[4] = -2*cosw;        // a1
            a0 = 1 + (alpha/a);
        } 
        break;
        // Lowpass Filter
        case LPF:
        {
            coeffs[0] = (1 - cosw)/2;   // b2
            coeffs[1] = (1 - cosw);     // b1
            coeffs[2] = (1 - cosw)/2;   // b0
            coeffs[3] = 1 - alpha;      // a2
            coeffs[4] = -2*cosw;        // a1
            a0 = 1 + alpha;
        break;
        }
        // Allpass Filter
        case APF:
        {
            coeffs[0] = 1 + alpha;      // b2
            coeffs[1] = -2*cosw;        // b1
            coeffs[2] = 1 - alpha;      // b0
            coeffs[3] = 1 - alpha;      // a2
            coeffs[4] = -2*cosw;        // a1
            a0 = 1 + alpha;
        }  
        break;
        // Bandpass Filter
        case BPF:
        {
            coeffs[0] = -sinw/2;        // b2
            coeffs[1] = 0;              // b1
            coeffs[2] = sinw/2;         // b0
            coeffs[3] = 1 - alpha;      // a2
            coeffs[4] = -2*cosw;        // a1
            a0 = 1 + alpha;
        }
        break;
        // Notch Filter
        case NOTCH:
        {
            coeffs[0] = 1;              // b2
            coeffs[1] = -2*cosw;        // b1
            coeffs[2] = 1;              // b0
            coeffs[3] = 1 - alpha;      // a2
            coeffs[4] = -2*cosw;        // a1
            a0 = 1 + alpha;
        }  
        break;
        // Highshelf filter
        case HIGHSHELF:
        {
            double inv_slope = (((1-2*q*q) * a) / q*q*(a*a+1)) + 1;

            coeffs[0] = a*((a + 1) + (a - 1)*cosw - sinw * sqrt(((a*a + 1) * (inv_slope - 1)) + 2*a));     // b2
            coeffs[1] = -2*a*((a - 1) + (a + 1)*cosw);                                                 // b1
            coeffs[2] =  a*((a + 1) + (a - 1)*cosw + sinw * sqrt(((a*a + 1) * (inv_slope - 1)) + 2*a));   // b0
            coeffs[3] = ((a + 1) - (a - 1)*cosw - sinw * sqrt(((a*a + 1) * (inv_slope - 1)) + 2*a));      // a2
            coeffs[4] = 2*((a - 1) - (a + 1)*cosw);                                                    // a1
            a0 = ((a + 1) - (a - 1)*cosw + sinw * sqrt(((a*a + 1) * (inv_slope - 1)) + 2*a));
        }
        break;
        // Highpass Filter
        case HPF:
        default:
        {
            coeffs[0] = (1 + cosw)/2;   // b2
            coeffs[1] = -1 - cosw;      // b1
            coeffs[2] = (1 + cosw)/2;   // b0
            coeffs[3] = 1 - alpha;      // a2
            coeffs[4] = -2*cosw;        // a1
            a0 = 1 + alpha;
        } 
        break;
    }

    // Normalize with respec to a0
    for(int n = 0; n < NUMBER_OF_BIQUAD_COEFFICIENTS; n++){
        coeffs[n] /= a0;
    }
}

/**
 * @brief compute RMS of sample buffer, with output option set by flagLevel (either amplitude or dB)
 * 
*/
float compute_rms(float* input, unsigned size, int flagLevel)
{
    float sum = 0.0f;
    for(unsigned n = 0; n < size; n++){
        sum += (input[n]*input[n]);
    }

    if(flagLevel){
        sum = 10.0f*log10f(sum / (float) size);
    }
    else {
        sum =  sqrtf(sum / (float) size);
    }

    return sum;
}

/**
 * @brief compute FFT of sample buffer
 * Not being used at the moment, but computation load was tested in the past 
 * and maybe it will used for future implementation... 
*/
void compute_fft(float *input, unsigned bufferSize){
    // printf("\nFFT function called");
    double *in;
    unsigned n;
    fftw_complex *intern;
    fftw_plan p;
    in = fftw_alloc_real(bufferSize);
    intern = fftw_alloc_complex(bufferSize);
    
    /* fill in complex input buffers */
    for(n = 0; n < bufferSize; n++){
        in[n] = (double) input[n];
    }

    /* FFT */
    p = fftw_plan_dft_r2c_1d(bufferSize, in, intern, FFTW_ESTIMATE);
    fftw_execute(p); /* repeat as needed */
    fftw_destroy_plan(p);
    
    /* FFT normalization */
    for(n = 0; n < bufferSize; n++){
        intern[n][0] /= bufferSize;
        intern[n][1] /= bufferSize;
    }    

    fftw_free(in);
    fftw_free(intern);
}

/**
 * @brief initialize biquad filter (biquad_filter_data)
 * 
*/
void init_filter(biquad_filter_data* filter){
    // allocate memory for filter coefficients
    filter->coeffs = malloc(NUMBER_OF_BIQUAD_COEFFICIENTS * sizeof(double));
    // compute filter coefficients
    compute_biquad_filter_coeffs(filter->coeffs, 
                                 filter->filterType, 
                                 filter->cutoffFrequency, 
                                 filter->qFactor,
                                 filter->gain, 
                                 filter->sampleRate);
    // allocate memory for delayed/computed samples
    filter->previousInput = calloc(2, sizeof(float));
    filter->previousOutput = calloc(2, sizeof(float));
}

/**
 * @brief free biquad filter (biquad_filter_data)
 * 
*/
void free_filter(biquad_filter_data* filter){
    // free HPF pointers
    free(filter->coeffs);
    free(filter->previousInput);
    free(filter->previousOutput);
}