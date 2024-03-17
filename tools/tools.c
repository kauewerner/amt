/*
--------------------------------------------------------------------------
* Acoustic monitoring tool for Raspberry Pi based on miniaudio framework *
*                                                                        *
* Kaue Werner, 2024                                                      *
--------------------------------------------------------------------------
*/
/**
 * @file tools.c
 * @author Kaue Werner
 * @date 10 Mar 2024
 * @brief Definition of general tool functions used in AMT
 * @version 0.1.0
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "tools.h"
#include <string.h>

/**
 * @brief set AMT configuration struct fields based on 
 * data read from input amt.config
 *
*/
void set_config(const char* configFile, amt_config* config){
    char line[TMP_CHAR_SIZE];
    char label[TMP_CHAR_SIZE];
    char stringValue[TMP_CHAR_SIZE];
    int numberValue;
    
    FILE* file = fopen(configFile, "r");
    while(!feof(file))
    {
        fgets(line, sizeof(line), file);
        sscanf(line, "%s[^\t]", label);

        if(!strcmp(label, "microphoneGain")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->microphoneGain = (float) numberValue;
        #ifdef DEBUG
            printf("%s = %.1f\n", label, config->microphoneGain);
        #endif
            continue;
        }

        if(!strcmp(label, "recordDuration")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->recordDuration = (float) numberValue;
        #ifdef DEBUG
            printf("%s = %.1f\n", label, config->recordDuration);
        #endif
            continue;
        }

        if(!strcmp(label, "sleepDuration")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->sleepDuration = (float) numberValue;
        #ifdef DEBUG
            printf("%s = %.1f\n", label, config->sleepDuration);
        #endif
            continue;
        }

        if(!strcmp(label, "recordingHours")){
            sscanf(line, "%s\t%s\n", label, stringValue);
            if(strcmp(stringValue, "-")){
                unsigned tmpHours[TMP_CHAR_SIZE];
                unsigned count = 0;
                unsigned twoDigitFlag = 0;
                for(int n = 0; n < TMP_CHAR_SIZE; n++){
                    if( stringValue[n] == '.' ){
                        break;
                    }
                    if ( stringValue[n] == ',' ){
                        continue;
                    }
                    if ( twoDigitFlag ){
                        twoDigitFlag = 0;
                        continue;
                    }
                    if( stringValue[n+1] == ',' ||  stringValue[n+1] == '.' ){
                        tmpHours[count] = (((int) stringValue[n]) - ZERO_CHAR_AS_INT );
                        count++;
                    }
                    else {
                        twoDigitFlag = 1;
                        tmpHours[count] = (((int) stringValue[n]) - ZERO_CHAR_AS_INT )*10 + (((int) stringValue[n+1]) - ZERO_CHAR_AS_INT );
                        count++;
                        continue;
                    }
                }
            #ifdef DEBUG
                printf("Recording hours of the day:\n");
            #endif
                config->recordingHours = malloc(count * sizeof(unsigned));
                for(int n = 0; n < count; n++){
                    config->recordingHours[n] = tmpHours[n];
                #ifdef DEBUG
                    printf("%d:00\n", config->recordingHours[n]);
                #endif
                }
                config->numberOfRecordingHours = count;
            }
            else {
            #ifdef DEBUG
                printf("Recording hours of the day:\n");
            #endif
                config->recordingHours = malloc(24 * sizeof(unsigned));
                for(unsigned n = 0; n < 24; n++){
                    config->recordingHours[n] = n;
                #ifdef DEBUG
                    printf("%d:00\n", config->recordingHours[n]);
                #endif
                }
                config->numberOfRecordingHours = 24;
            }
            continue;
        }

        if(!strcmp(label, "firstRecordingDate")){
            sscanf(line, "%s\t%s\n", label, stringValue);
            config->firstRecordingDate = malloc(DATE_ARRAY_SIZE * sizeof(char));
            for(int n = 0; n < DATE_ARRAY_SIZE; n++){
                config->firstRecordingDate[n] = stringValue[n];
            }
        #ifdef DEBUG
            printf("%s = %s\n", label, config->firstRecordingDate);
        #endif
            continue;
        }

        if(!strcmp(label, "lastRecordingDate")){
            sscanf(line, "%s\t%s\n", label, stringValue);
            // config->lastRecordingDate = stringValue;
            config->lastRecordingDate = malloc(DATE_ARRAY_SIZE * sizeof(char));
            for(int n = 0; n < DATE_ARRAY_SIZE; n++){
                config->lastRecordingDate[n] = stringValue[n];
            }
        #ifdef DEBUG
            printf("%s = %s\n", label, config->lastRecordingDate);
        #endif
            continue;
        }

        if(!strcmp(label, "sampleRate")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->sampleRate = (float) numberValue;
        #ifdef DEBUG
            printf("%s = %.1f\n", label, config->sampleRate);
        #endif
            continue;
        }

        if(!strcmp(label, "enableHighpassFilter")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->enableHighpassFilter = (unsigned) numberValue;
        #ifdef DEBUG
            printf("%s = %d\n", label, config->enableHighpassFilter);
        #endif
            continue;
        }

        if(!strcmp(label, "highpassFilterCutoff")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->highpassFilterCutoff = (float) numberValue;
        #ifdef DEBUG
            printf("%s = %.1f\n", label, config->highpassFilterCutoff);
        #endif
            continue;
        }

        if(!strcmp(label, "enableLowpasssFilter")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->enableLowpasssFilter = (unsigned) numberValue;
        #ifdef DEBUG
            printf("%s = %d\n", label, config->enableLowpasssFilter);
        #endif
            continue;
        }

        if(!strcmp(label, "lowpassFilterCutoff")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->lowpassFilterCutoff = (float) numberValue;
        #ifdef DEBUG
            printf("%s = %.1f\n", label, config->lowpassFilterCutoff);
        #endif
            continue;
        }

        if(!strcmp(label, "enableThresholdRecording")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->enableThresholdRecording = (unsigned) numberValue;
        #ifdef DEBUG
            printf("%s = %d\n", label, config->enableThresholdRecording);
        #endif
            continue;
        }

        if(!strcmp(label, "recordingThresholddBFS")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->recordingThresholddBFS = (float) numberValue;
        #ifdef DEBUG
            printf("%s = %.1f\n", label, config->recordingThresholddBFS);
        #endif
            continue;
        }

        if(!strcmp(label, "recordedTimeBeforeThreshold")){
            sscanf(line, "%s\t%d\n", label, &numberValue);
            config->recordedTimeBeforeThreshold = (float) numberValue;
        #ifdef DEBUG
            printf("%s = %.1f\n", label, config->recordedTimeBeforeThreshold);
        #endif
            continue;
        }
        
    }
    fclose(file);
}

/**
 * @brief function used to update output wav file name
 *
*/
void update_output_file_name(char * ptr, unsigned size)
{
    time_t rawtime;
    struct tm *info;
    time( &rawtime );
    info = localtime( &rawtime );
    strftime(ptr, size, OUTPUT_WAV_FILE_LABEL, info);
}

/**
 * @brief get current hour extracted from from current date
 *
*/
int get_current_hour(){
    time_t now = time(NULL);
    struct tm *tm_struct = localtime(&now);
    return tm_struct->tm_hour;
}

/**
 * @brief get current minute extracted from from current date
 *
*/
int get_current_minute(){
    time_t now = time(NULL);
    struct tm *tm_struct = localtime(&now);
    return tm_struct->tm_min;
}

/**
 * @brief update date pointer with current date
 *
*/
void update_date(char * ptr, unsigned size)
{
    time_t rawtime;
    struct tm *info;
    time( &rawtime );
    info = localtime( &rawtime );
    strftime(ptr, size, DATE_LABEL, info);
}

/**
 * @brief get current date
 *
*/
char* get_current_date_time() {
    time_t t;
    time(&t);
    return ctime(&t);
}

/**
 * @brief extract specific predefined info from an input date format pointer
 *
*/
unsigned extract_info_from_date(char* date, amt_date info){
    unsigned firstDigitIndex;
    switch (info){
        case MONTH:
            firstDigitIndex = DATE_MONTH_FIRST_DIGIT_INDEX;
        break;

        case DAY:
        default:
            firstDigitIndex = DATE_DAY_FIRST_DIGIT_INDEX;
        break;
    }
    return ( (((unsigned)date[firstDigitIndex]) - ZERO_CHAR_AS_INT) * 10 ) + (((unsigned)date[firstDigitIndex + 1]) - ZERO_CHAR_AS_INT);

}

/**
 * @brief check if current hour is in the list of recording hours
 *
*/
unsigned check_recording_hours(int currentHour, unsigned* recHours, unsigned numRecHours){
    unsigned isRecHour = 0;
    for(unsigned n = 0; n < numRecHours; n++){
        if(currentHour == recHours[n]){
            isRecHour = 1;
            break;
        }
    }
    return isRecHour;
}

