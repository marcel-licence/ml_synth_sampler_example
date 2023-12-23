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
 * Dieses Programm ist Freie Software: Sie k�nnen es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * ver�ffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * Dieses Programm wird in der Hoffnung bereitgestellt, dass es n�tzlich sein wird, jedoch
 * OHNE JEDE GEW�HR,; sogar ohne die implizite
 * Gew�hr der MARKTF�HIGKEIT oder EIGNUNG F�R EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License f�r weitere Einzelheiten.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

/**
 * @file fs_esp32.cpp
 * @author Marcel Licence
 * @data 27.06.2023
 *
 * @brief   This file contains ESP32 sepecific file system imeplementation
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef ESP32


/*
 * includes
 */
#include "fs_access.h"
#include "ml_soundfont.h"
#include "ml_sampler.h"


#include <FS.h>
#ifdef ARDUINO_RUNNING_CORE /* tested with arduino esp32 core version 2.0.2 */
#include <LittleFS.h> /* Using library LittleFS at version 2.0.0 from https://github.com/espressif/arduino-esp32 */
#else
#include <LITTLEFS.h> /* Using library LittleFS_esp32 at version 1.0.6 from https://github.com/lorol/LITTLEFS */
#define LittleFS LITTLEFS
#endif
#include <SD_MMC.h>


/*
 * definitions
 */
#define FORMAT_LITTLEFS_IF_FAILED true
#define Status_LogMessage(a) Serial.printf(a);Serial.printf("\n")


/*
 * static function declarations
 */
static void printDirectory(File dir, int numTabs);
static bool FS_SdCardInit(void);


/*
 * static function definitions
 */
static void printDirectory(File dir, int numTabs)
{
    while (true)
    {

        File entry = dir.openNextFile();
        if (! entry)
        {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}


/*
 * extern function definitions
 */
static bool FS_SdCardInit(void)
{
#ifdef ESP32
    if (!SD_MMC.begin("/sdcard", true))
#else
    if (!card.init(SD_DETECT_NONE))
#endif
    {
        Status_LogMessage("Card Mount Failed");
        delay(1000);
        return false;
    }

    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE)
    {
        Status_LogMessage("No SD card attached");

        delay(1000);
        return false;
    }

    if (cardType == CARD_MMC)
    {
        Status_LogMessage("Card Access: MMC");
    }
    else if (cardType == CARD_SD)
    {
        Status_LogMessage("Card Access: SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Status_LogMessage("Card Access: SDHC");
    }
    else
    {
        Status_LogMessage("Card Access: UNKNOWN");
    }

    {
        File dir = SD_MMC.open("/");
        printDirectory(dir, 0);
        dir.close();
    }
    {
        File dir = SD_MMC.open("/");
        printDirectory(dir, 0);
        dir.close();
    }
    {
        File dir = SD_MMC.open("/tje_drum/");

        Serial.printf("Showing dir: %s\n", "/tje_drum/");

        printDirectory(dir, 0);
        dir.close();
    }
    {
        File dir = SD_MMC.open("tje_drum/");
        Serial.printf("Showing dir: %s\n", "tje_drum/");
        printDirectory(dir, 0);
        dir.close();
    }
    {
        File dir = SD_MMC.open("/tje_drum");
        Serial.printf("Showing dir: %s\n", "/tje_drum");
        printDirectory(dir, 0);
        dir.close();
    }
    {
        File dir = SD_MMC.open("tje_drum");
        Serial.printf("Showing dir: %s\n", "tje_drum");
        printDirectory(dir, 0);
        dir.close();
    }

    return true;
}

static void FS_LittleFsInit(void)
{
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
    {
        Serial.printf("LittleFS Mount Failed");
        return ;
    }

    Serial.printf("LittleFS Mount Successful!\n");

    {
        File dir = LittleFS.open("/");
        printDirectory(dir, 0);
        dir.close();
    }

    {
        File dir = LittleFS.open("/tje_drum/");
        printDirectory(dir, 0);
        dir.close();
    }
}


/*
 * extern function definitions
 */
void FS_Setup(void)
{
    FS_SdCardInit();
    FS_LittleFsInit();

    PatchManager_GetFileList(FS_ID_LITTLEFS, "/", 0, 10);
    PatchManager_GetFileList(FS_ID_SD_MMC, "/", 0, 10);
}


#endif /* ESP32 */

