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
 * @file fs_access.cpp
 * @author Marcel Licence
 * @data 20.10.2023
 *
 * @brief   File contains implementation of different file accessors
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#include "fs_access.h"
#include "utils.h"

#include <FS.h>
#if (defined ARDUINO_RUNNING_CORE) || (defined ARDUINO_RASPBERRY_PI_PICO) || (defined ARDUINO_ARCH_RP2040) /* tested with arduino esp32 core version 2.0.2 */
#include <LittleFS.h> /* Using library LittleFS at version 2.0.0 from https://github.com/espressif/arduino-esp32 */
#else
#include <LITTLEFS.h> /* Using library LittleFS_esp32 at version 1.0.6 from https://github.com/lorol/LITTLEFS */
#define LittleFS LITTLEFS
#endif

#ifdef ESP32
#include <SD_MMC.h>
#endif

/*
 * defines
 */
#define FST fs::FS

/*
 * function declarations
 */
static bool FS_OpenFile(FST &fs, const char *filename);
static void PatchManager_GetFileList(fs::FS &fs, const char *dirname, void(*fileInd)(fs::FS &fs, File *file, const char *filename, int offset, uint8_t note), int depth, int max_depth);

/*
 * local variables
 */
static File f;
static File *g_file = NULL;
static File *t_file = NULL;


static FST &FsFromId(fs_id_t id)
{
    switch (id)
    {
    case FS_ID_LITTLEFS:
        return LittleFS;
#ifdef ESP32
    case FS_ID_SD_MMC:
        return SD_MMC;
#endif
    }
    return LittleFS;
}

bool FS_OpenFile(fs_id_t id, const char *filename)
{
    return FS_OpenFile(FsFromId(id), filename);
}

static bool FS_OpenFile(FST &fs, const char *filename)
{
#ifdef ESP32
    f = fs.open(filename);
#else
    f = fs.open(filename, "r");
#endif
    if (f)
    {
        g_file = &f;
        return true;
    }
    else
    {
        Serial.printf("Error opening file: %s\n", filename);
        return false;
    }
}

void FS_LittleFS_CloseFile(void)
{
    g_file->close();
    g_file = NULL;
}

void FS_CloseFile(void)
{
    g_file->close();
    g_file = NULL;
}


void WavToKeyboard(fs_id_t id, const char *dirname, void(*fileInd)(const char *filename, int offset, uint8_t note), int depth, int max_depth, uint8_t note)
{
#ifdef ESP32
    File root = FsFromId(id).open(dirname);
#else
    File root = FsFromId(id).open(dirname, "r");
#endif
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    Serial.printf("WavToKeyboard root:%s\n", root.name());

    File file = root.openNextFile();
    if (!file)
    {
        Serial.printf("no files in root!\n");
    }
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.printf("Dir Found: %s (ignoreD)\n", file.name());
            if (max_depth != 0)
            {
                // PatchManager_GetFileList(fs, file.name(), fileInd, depth + 1, max_depth - 1);
            }
        }
        else
        {
            Serial.printf("File Found: %s\n", file.name());
            t_file = & file;
            fileInd(file.name(), depth, note++);
        }
        file = root.openNextFile();
    }

}

void FS_UseTempFile(void)
{
    g_file = t_file;
}

uint32_t readBytes(uint8_t *buffer, uint32_t len)
{
    return g_file->read(buffer, len);
}

void fileSeekTo(uint32_t pos)
{
    g_file->seek(0, SeekSet);
    g_file->seek(pos, SeekSet);
}

uint32_t getStaticPos(void)
{
    return g_file->position();
}

uint32_t getCurrentOffset(void)
{
    return g_file->position();
}

uint32_t readBytesFromAddr(uint8_t *buffer, uint32_t addr, uint32_t len)
{
    fileSeekTo(addr);
    return readBytes(buffer, len);
}

void PatchManager_GetFileList(fs_id_t fs_id, const char *dirname, int depth, int max_depth)
{
    PatchManager_GetFileList(FsFromId(fs_id), dirname, NULL, depth, max_depth);
}

static void PatchManager_GetFileList(fs::FS &fs, const char *dirname, void(*fileInd)(fs::FS &fs, File *file, const char *filename, int offset, uint8_t note), int depth, int max_depth)
{
    Serial.printf("PatchManager_GetFileList: %s\n", dirname);
#ifdef ESP32
    File root = fs.open(dirname);
#else
    File root = fs.open(dirname, "r");
#endif
    if (!root)
    {
        Serial.printf("- failed to open directory: %s\n", dirname);
        return;
    }
    if (!root.isDirectory())
    {
        /* should never enter here */
        Serial.printf(" - not a directory: %s\n", dirname);
        return;
    }

    Serial.printf("PatchManager_GetFileList root:%s\n", root.name());

    File file = root.openNextFile();
    if (!file)
    {
        Serial.printf("no files in root!\n");
    }
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.printf("Dir Found:  %s%s\n", dirname, file.name());
            if (max_depth != 0)
            {
                char dirName[256];
                sprintf(dirName, "%s%s/", dirname, file.name());
                PatchManager_GetFileList(fs, dirName, fileInd, depth + 1, max_depth - 1);
            }
        }
        else
        {
            Serial.printf("File Found: %s%s\n", dirname, file.name());
            if (fileInd != NULL)
            {
                fileInd(fs, &file, file.name(), depth, 0xFF);
            }
        }
        file = root.openNextFile();
    }
}

