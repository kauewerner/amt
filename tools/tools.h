/*
--------------------------------------------------------------------------
* Acoustic monitoring tool for Raspberry Pi based on miniaudio framework *
*                                                                        *
* Kaue Werner, 2024                                                      *
--------------------------------------------------------------------------
*/
/**
 * @file tools.h
 * @author Kaue Werner
 * @date 10 Mar 2024
 * @brief Header with general tool functions used in AMT
 * @version 0.1.0
*/
#ifndef TOOLS_H
#define TOOLS_H
#include "../config_defines.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @brief AMT main configuration data struct
 *
*/
typedef struct{
    /* fields related to amt.config input data */
    float microphoneGain;
    float recordDuration;
    float sleepDuration;
    unsigned* recordingHours;
    char* firstRecordingDate;
    char* lastRecordingDate;
    float sampleRate;
    unsigned enableHighpassFilter:1;
    float highpassFilterCutoff;
    unsigned enableLowpasssFilter:1;
    float lowpassFilterCutoff;
    unsigned enableThresholdRecording:1;
    float recordingThresholddBFS;
    float recordedTimeBeforeThreshold;
    /* internal usage fields */
    unsigned numberOfRecordingHours;
    float micGainFactor;
} amt_config;

/**
 * @brief AMT data format enum
 *
*/
typedef enum {
	DAY,
    MONTH
} amt_date;

/**
 * @brief set AMT configuration struct fields based on 
 * data read from input amt.config
 *
*/
void set_config(const char* configFile, amt_config* config);

/**
 * @brief function used to update output wav file name
 *
*/
char* get_current_date_time();

/**
 * @brief get current hour extracted from from current date
 *
*/
void update_output_file_name(char * buffer, unsigned size);

/**
 * @brief get current minute extracted from from current date
 *
*/
int get_current_hour();

/**
 * @brief update date pointer with current date
 *
*/
int get_current_minute();

/**
 * @brief get current date
 *
*/
void update_date(char * ptr, unsigned size);

/**
 * @brief extract specific predefined info from an input date format pointer
 *
*/
unsigned extract_info_from_date(char* date, amt_date info);

/**
 * @brief check if current hour is in the list of recording hours
 *
*/
unsigned check_recording_hours(int currentHour, unsigned* recHours, unsigned numRecHours);
#endif