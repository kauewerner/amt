/*
--------------------------------------------------------------------------
* Acoustic monitoring tool for Raspberry Pi based on miniaudio framework *
*                                                                        *
* Kaue Werner, 2024                                                      *
--------------------------------------------------------------------------
*/
/**
 * @file main.c
 * @author Kaue Werner
 * @date 10 Mar 2024
 * @brief Main file of Acoustic Monitoring Tool (AMT)
 * @version 0.1.0
*/
#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"
#include "config_defines.h"
#include "tools/tools.h"
#include "audio_proc/audio_proc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// struct used to create rec dir if non existent
struct stat st = {0};

// global miniaudio IO variables 
ma_result result;
ma_device_config deviceConfig;
ma_device device;
ma_encoder_config encoderConfig;
ma_encoder encoder;

// path to input configuration file where parameters are read
const char * configFileName = CONFIG_FILE_PATH;

// temp output file name to be updated for each recording
char tmpOutputFileName[MAX_CHAR_LENGTH];

// current date to be updated while running
char currentDate[MAX_CHAR_LENGTH];

// log file to write recording log
FILE *logFile;

// structure with recording flags used to recording start/stop management
typedef struct {
    unsigned initialized:1;
    unsigned ongoing:1;
    unsigned filledDataBeforeThreshold:1;
} recording_flags;

// structure with flags used for audio IO management
typedef struct {
    unsigned initialized:1;
    unsigned ongoing:1;
    unsigned finished:1;
} audio_io_flags;

// global recording flag struct pointer
recording_flags *recFlags;

// global recording frame counter
unsigned recCounter = 0;

// recording buffer to store samples before starting threshold is reached
float* recordingBufferBeforeThreshold;

// pointer to High Pass Filter
biquad_filter_data* hpf; 
// pointer to Low Pass Filter
biquad_filter_data* lpf; 

// global audio IO flag struct pointer
audio_io_flags* audioIoFlags;

// global configuration struct
amt_config* amtConfig;

// Specific callback function format to be used with miniaudio as default IO framework
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    // copy input to filter buffer and apply mic gain
    float filteredInput[NUMBER_OF_CALLBACK_SAMPLES];
    for(unsigned n = 0; n < NUMBER_OF_CALLBACK_SAMPLES; n++){  	    
        filteredInput[n] = ((float *) pInput)[n];
        filteredInput[n] *= amtConfig->micGainFactor;
    }
    // apply HPF to buffer data
    if(hpf){
        process_filter(filteredInput, (unsigned) frameCount, 
                     hpf->previousInput, hpf->previousOutput, hpf->coeffs);
    }
    // apply LPF to buffer data
    if(lpf){
        process_filter(filteredInput, (unsigned) frameCount, 
                     lpf->previousInput, lpf->previousOutput, lpf->coeffs);
    }

    // check if threshold-based recording is enabled, if not got to rec hours method
    if(amtConfig->enableThresholdRecording){
        unsigned recTimeInSamplesBeforeThreshold = (unsigned)(amtConfig->recordedTimeBeforeThreshold * amtConfig->sampleRate);
        if(!recFlags->ongoing){
        // update recording buffer before reaching threshold
        for(unsigned n = 0; n < recTimeInSamplesBeforeThreshold; n++){
            if(n < (recTimeInSamplesBeforeThreshold - NUMBER_OF_CALLBACK_SAMPLES)){
                recordingBufferBeforeThreshold[n] = recordingBufferBeforeThreshold[n + NUMBER_OF_CALLBACK_SAMPLES];
            }
            else {
                recordingBufferBeforeThreshold[n] = filteredInput[n - (recTimeInSamplesBeforeThreshold - NUMBER_OF_CALLBACK_SAMPLES)];
            } 
        }
        }
        // compute dB RMS of the current buffer
        float currentRMS = compute_rms(filteredInput, frameCount, 1);

        if(currentRMS >= amtConfig->recordingThresholddBFS && !recFlags->ongoing){
            recFlags->initialized = 1;
        }

        if(recFlags->initialized){
            update_output_file_name(tmpOutputFileName, MAX_CHAR_LENGTH);

            if (ma_encoder_init_file(tmpOutputFileName, &encoderConfig, &encoder) != MA_SUCCESS) {
                printf("Failed to initialize output file.\n");
            }

            recFlags->initialized = 0;
            recFlags->ongoing = 1;
            // open log file
            char logFileName[MAX_CHAR_LENGTH] = LOG_FILE_PATH;
            logFile = fopen(strcat(strcat(logFileName, currentDate), ".txt"), "a");
        #ifdef DEBUG
            printf("New recording started due to RMS level = %.2f...\n",currentRMS);
        #endif
            fprintf(logFile, "RMS level = %.2fdBFS\t", currentRMS);
            fprintf(logFile, "%s", get_current_date_time());
            
        }

        if(recFlags->ongoing){
            if(!recFlags->filledDataBeforeThreshold){
                ma_encoder_write_pcm_frames(&encoder, recordingBufferBeforeThreshold, recTimeInSamplesBeforeThreshold, NULL);
                recFlags->filledDataBeforeThreshold = 1;
            } else {
                if(recCounter < ((int)(amtConfig->sampleRate * amtConfig->recordDuration * 60) - recTimeInSamplesBeforeThreshold)){
                    ma_encoder_write_pcm_frames(&encoder, filteredInput, frameCount, NULL);
                    recCounter += frameCount;
                }
                else {
                    recCounter = 0;
                    recFlags->ongoing = 0;
                    recFlags->filledDataBeforeThreshold = 0;
                #ifdef DEBUG
                    printf("...recording finished!\n");
                #endif
                    fclose(logFile);
                    ma_encoder_uninit(&encoder);
                }
            }
        }
    } 
    else {
        if(!recFlags->ongoing && !audioIoFlags->finished){
            update_output_file_name(tmpOutputFileName, MAX_CHAR_LENGTH);
        #ifdef DEBUG
            printf("-> Updated rec output file name: %s\n", tmpOutputFileName);
        #endif
            if (ma_encoder_init_file(tmpOutputFileName, &encoderConfig, &encoder) != MA_SUCCESS) {
                printf("Failed to initialize output file.\n");
            }
            recFlags->ongoing = 1;
            // open log file
            char logFileName[MAX_CHAR_LENGTH] = LOG_FILE_PATH;
            logFile = fopen(strcat(strcat(logFileName, currentDate), ".txt"), "a");
        #ifdef DEBUG
            printf("New recording started...\n");
            printf("-> Rec duration: %.2f min\n", amtConfig->recordDuration);
        #endif
            fprintf(logFile, "Rec initialized at ");
            fprintf(logFile, "%s", get_current_date_time());
        }

        if(recFlags->ongoing){
            if(recCounter < (int)(amtConfig->sampleRate * amtConfig->recordDuration * 60)){
                ma_encoder_write_pcm_frames(&encoder, filteredInput, frameCount, NULL);
                recCounter += frameCount;
            }
            else {
                recCounter = 0;
                recFlags->ongoing = 0;
            #ifdef DEBUG
                printf("...recording finished!\n");
            #endif
                fclose(logFile);
                ma_encoder_uninit(&encoder);
                audioIoFlags->finished = 1;
            }
        }
    }
    (void)pOutput;
}

void init_audio_io(){
    // init recording flags as zero
    recFlags = malloc(sizeof(recording_flags));
    recFlags->initialized = 0;
    recFlags->ongoing = 0;
    recFlags->filledDataBeforeThreshold = 0;

    if(amtConfig->enableThresholdRecording){
        // init past samples recording buffer
        recordingBufferBeforeThreshold = calloc((int)(amtConfig->recordedTimeBeforeThreshold * amtConfig->sampleRate), sizeof(float));
    }

    // init miniaudio encoder config
    encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, ma_format_f32, NUMBER_OF_INPUT_CHANNELS, (ma_uint32) amtConfig->sampleRate);

    // init miniaudio device config
    deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.format   = ma_format_f32;   
    deviceConfig.capture.channels = NUMBER_OF_INPUT_CHANNELS;
    deviceConfig.sampleRate       = (ma_uint32) amtConfig->sampleRate;
    deviceConfig.periodSizeInFrames = NUMBER_OF_CALLBACK_SAMPLES;
    deviceConfig.dataCallback     = data_callback;

    // init miniaudio device
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize capture device.\n");
    }

    // start miniaudio device
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&device);
        printf("Failed to start device.\n");
    }

}

void fini_audio_io(){
    // uninit miniaudio device
    ma_device_uninit(&device);

    // free recording flags
    free(recFlags);

    // free past samples buffer
    free(recordingBufferBeforeThreshold);
}


int main(int argc, char** argv)
{
    // Init audio IO flags
    audioIoFlags = malloc(sizeof(audio_io_flags));
    audioIoFlags->initialized = 0;
    audioIoFlags->ongoing = 0;
    audioIoFlags->finished = 0;

    // Create a recording dir if non-existent
    if (stat(REC_DIR, &st) == -1) {
    #ifdef PC_TEST
        mkdir(REC_DIR);
    #else
        mkdir(REC_DIR, 0777);
    #endif
    }

    // Get current date
    update_date(currentDate, MAX_CHAR_LENGTH);
#ifdef DEBUG
    printf("-> Current date: %s\n", currentDate);
#endif

    // Set main configuration settings
    amtConfig = malloc(sizeof(amt_config));
    set_config(configFileName, amtConfig);

    // Compute mic gain factor
    amtConfig->micGainFactor = powf(10.0f, amtConfig->microphoneGain / 20.0f);

    // Init HPF
    if(amtConfig->enableHighpassFilter){
    #ifdef DEBUG
        printf("HPF enabled!\n");
    #endif
        hpf = malloc(sizeof(biquad_filter_data));
        hpf->filterType = HPF;
        hpf->qFactor = HPF_Q_FACTOR;
        hpf->gain = 0.0;
        hpf->cutoffFrequency = amtConfig->highpassFilterCutoff;
        hpf->sampleRate = amtConfig->sampleRate;
        init_filter(hpf);
    }

    // Init LPF
    if(amtConfig->enableLowpasssFilter){
    #ifdef DEBUG
        printf("LPF enabled!\n");
    #endif
        lpf = malloc(sizeof(biquad_filter_data));
        lpf->filterType = LPF;
        lpf->qFactor = HPF_Q_FACTOR;
        lpf->gain = 0.0;
        lpf->cutoffFrequency = amtConfig->lowpassFilterCutoff;
        lpf->sampleRate = amtConfig->sampleRate;
        init_filter(lpf);
    }

    // First check current date, if not in the firstRecordingDate, sleep until there
    unsigned runningFlag = 0;
    unsigned currentDay, currentMonth;
    unsigned firstRecordingDay = extract_info_from_date(amtConfig->firstRecordingDate, DAY);
    unsigned lastRecordingDay = extract_info_from_date(amtConfig->lastRecordingDate, DAY);
    unsigned firstRecordingMonth = extract_info_from_date(amtConfig->firstRecordingDate, MONTH);
    unsigned lastRecordingMonth = extract_info_from_date(amtConfig->lastRecordingDate, MONTH);
    while(!runningFlag){
        // Get day and month of current date
        currentDay = extract_info_from_date(currentDate, DAY);
        currentMonth = extract_info_from_date(currentDate, MONTH);
    #ifdef DEBUG
        printf("Current day: %d\n", currentDay);
        printf("Current month: %d\n", currentMonth);
        printf("First recording day: %d\n", firstRecordingDay);
    #endif
        if(currentMonth >= firstRecordingMonth){
            if(currentDay >= firstRecordingDay){
            #ifdef DEBUG
                printf("Today is the starting day!\n");
            #endif
                runningFlag = 1;
            } 
            else {
            #ifdef DEBUG
                printf("Today is not the starting day! Checking again after %d minute(s)...\n", DATE_CHECK_TIME_IN_MINUTES);
            #endif
            #ifdef PC_TEST
                Sleep((int)(DATE_CHECK_TIME_IN_MINUTES * 60 * 1000));
            #else
                sleep((int)(DATE_CHECK_TIME_IN_MINUTES * 60));
            #endif
            }
        }
        else {
            #ifdef DEBUG
                printf("Today is not the starting month! Checking again after 1 day...\n");
            #endif
            #ifdef PC_TEST
                Sleep((int)(DATE_CHECK_TIME_IN_MINUTES * 60 * 60 * 24 * 1000));
            #else
                sleep((int)(DATE_CHECK_TIME_IN_MINUTES * 60 * 60 * 24));
            #endif
        } 
    }
    
    // If threshold-based rec mode is not enabled, start recs without threshold mode
    if(!amtConfig->enableThresholdRecording){
        while(runningFlag) {   
            if(!audioIoFlags->initialized){
                if(check_recording_hours(get_current_hour(),amtConfig->recordingHours,amtConfig->numberOfRecordingHours)){
                #ifdef DEBUG
                    printf("Current hour where recording starts: %d\n", get_current_hour());
                #endif
                    audioIoFlags->initialized = 1;
                    // initialize miniaudio
                    init_audio_io();
                } 
                else {
                    // if not recording hour, sleep and check again in 1 minute
                #ifdef DEBUG
                    printf("Current hour is not a recording hour! Checking again in 1min...");
                #endif
                #ifdef PC_TEST
                    Sleep((int)(60 * 1000));
                #else
                    sleep((int)(60));
                #endif
                }
            } else {
                if(audioIoFlags->finished)
                {
                    // finilize miniaudio
                    fini_audio_io();
                    audioIoFlags->finished = 0;
                    audioIoFlags->initialized = 0;
                #ifdef DEBUG
                    printf("Calling sleep function...\n");
                    printf("-> Sleep duration: %.2f min\n", amtConfig->sleepDuration);
                #endif
                #ifdef PC_TEST
                    Sleep((int)(amtConfig->sleepDuration * 60 * 1000));
                #else
                    sleep((int)(amtConfig->sleepDuration * 60));
                #endif
                #ifdef DEBUG
                    printf("...Sleep function finished\n");
                #endif

                    // check if current date is not later than last recording date
                    update_date(currentDate, MAX_CHAR_LENGTH);
                    currentMonth = extract_info_from_date(currentDate, MONTH);
                    if(currentMonth > lastRecordingMonth){
                        runningFlag = 0;
                    #ifdef DEBUG
                        printf("Stoping AMT since the last recording month is earlier than the current month...");
                    #endif
                    }
                    else{
                        if(currentMonth == lastRecordingMonth){
                            currentDay = extract_info_from_date(currentDate, DAY);
                            if(currentDay > lastRecordingDay){
                                runningFlag = 0;
                            #ifdef DEBUG
                                printf("Stoping AMT since the last recording day is earlier than the current day...");
                            #endif
                            }
                        }
                    }
                }
            }
        };
    } 
    else
    {
        // initialize miniaudio
        init_audio_io();
        while(runningFlag) {
            if(check_recording_hours(get_current_hour(),amtConfig->recordingHours,amtConfig->numberOfRecordingHours)){
                audioIoFlags->initialized = 1;
            } 
            else {
                #ifdef PC_TEST
                    Sleep((int)(60 * 1000));
                #else
                    sleep((int)(60));
                #endif
            }

            // check if current date is not later than last recording date
            update_date(currentDate, MAX_CHAR_LENGTH);
            currentMonth = extract_info_from_date(currentDate, MONTH);
            if(currentMonth > lastRecordingMonth){
                runningFlag = 0;
            }
            currentDay = extract_info_from_date(currentDate, DAY);
            if(currentDay > lastRecordingDay){
                runningFlag = 0;
            }
        };
        // finilize miniaudio
        fini_audio_io();
    }

    // free all memory allocation
    if(hpf){
        free_filter(hpf);
    }
    if(lpf){
        free_filter(lpf);
    }
    free(amtConfig->recordingHours);
    free(amtConfig->firstRecordingDate);
    free(amtConfig->lastRecordingDate);
    free(amtConfig);
    
    return 0;
}
