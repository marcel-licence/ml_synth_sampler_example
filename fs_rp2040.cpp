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
 * @file fs_rp2040.cpp
 * @author Marcel Licence
 * @data 27.06.2023
 *
 * @brief   This file contains RP2040 sepecific file system imeplementation
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef ARDUINO_ARCH_RP2040


/*
 * include files
 */
#include "sf_to_sampler.h"
#include <FS.h>
#include <LittleFS.h>


/*
 * definitions
 */
#define FST fs::FS


/*
 * function definitions
 */
void FS_Setup(void)
{
    LittleFS.begin();

    Serial.printf("List files ...\n\n");
    SF2ToSmpl_LoadAllSamplesFromSF(FS_ID_LITTLEFS, "/yoissh.sf2");
}


#endif

