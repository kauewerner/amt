/*
--------------------------------------------------------------------------
* Acoustic monitoring tool for Raspberry Pi based on miniaudio framework *
*                                                                        *
* Kaue Werner, 2024                                                      *
--------------------------------------------------------------------------
*/
/**
 * @file config_defines.h
 * @author Kaue Werner
 * @date 10 Mar 2024
 * @brief Header with preprocessor configuration defines used in the AMT build
 * @version 0.1.0
*/
#ifndef CONFIG_DEFINES_H
#define CONFIG_DEFINES_H

#define DEVICE_NAME "rpi0n1"

#define DEBUGx
#define PC_TESTx

#ifdef PC_TEST
#define CONFIG_FILE_PATH "./amt.config"
#define LOG_FILE_PATH "./recording_log_"
#define OUTPUT_WAV_FILE_DIR "./recs/"
#define OUTPUT_WAV_FILE_SUFFIX "_%Y-%m-%d_%H-%M-%S.wav"
#define REC_DIR "./recs"
#else
#define CONFIG_FILE_PATH "/home/pi/amt/amt.config"
#define LOG_FILE_PATH "/home/pi/amt/recording_log.txt"
#define OUTPUT_WAV_FILE_DIR "/home/pi/amt/recs/"
#define OUTPUT_WAV_FILE_SUFFIX "_%Y-%m-%d_%H-%M-%S.wav"
#define REC_DIR "/home/pi/amt/recs"
#endif

#define DATE_ARRAY_SIZE 10
#define DATE_CHECK_TIME_IN_MINUTES 1
#define DATE_DAY_FIRST_DIGIT_INDEX 8
#define DATE_MONTH_FIRST_DIGIT_INDEX 5
#define DATE_LABEL "%Y-%m-%d"
#define HPF_Q_FACTOR 0.707
#define MAX_CHAR_LENGTH 100
#define NUMBER_OF_BIQUAD_COEFFICIENTS 5
#define NUMBER_OF_CALLBACK_SAMPLES 256
#define NUMBER_OF_INPUT_CHANNELS 1
#define ZERO_CHAR_AS_INT 48

#endif
