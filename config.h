/*
 * Copyright (c) 2023 Marcel Licence
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * veröffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
 * OHNE JEDE GEWÄHR,; sogar ohne die implizite
 * Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Einzelheiten.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

/**
 * @file config.h
 * @author Marcel Licence
 * @data 03.06.2023
 *
 * @brief   This file contains the project configuration
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifndef CONFIG_H_
#define CONFIG_H_


#define STATUS_SIMPLE


#if (defined ARDUINO_RASPBERRY_PI_PICO) || (defined ARDUINO_ARCH_RP2040)
#define BOARD_ML_PICO_SYNTH
#endif

#ifdef ESP32
#define BOARD_ML_SYNTH_V2
//#define BOARD_ESP32_AUDIO_KIT_AC101
#endif

#include <ml_boards.h>

#undef ADC_ENABLED

#define SAMPLE_BUFFER_SIZE  48
#define SAMPLE_RATE 48000

//#define OUTPUT_SAW_TEST

#define SERIAL_BAUDRATE 115200

#if (defined ARDUINO_RASPBERRY_PI_PICO) || (defined ARDUINO_ARCH_RP2040)
#define SAMPLE_HDR_CNT  3
#define RP2040_AUDIO_PWM
#define MAX_DELAY_Q 24000
#define SAMPLER_STATIC_BUFFER_SIZE  (1024 * 16)
#endif

#define MIDI_FMT_INT


#ifdef ESP32
#define AUDIO_PASS_THROUGH
#define SAMPLE_HDR_CNT  32
#define SAMPLE_SIZE_16BIT
#define MAX_DELAY_Q 12000
//#define SAMPLER_STATIC_BUFFER_SIZE  512
//#define SAMPLER_DYNAMIC_BUFFER_SIZE (1024 * 16)
#define REVERB_ENABLED
#endif


#endif /* CONFIG_H_ */
