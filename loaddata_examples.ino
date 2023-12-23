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
 * @file loaddata_examples.ino
 * @author Marcel Licence
 * @data 20.10.2023
 *
 * @brief   Implementation of some examples loading samples from different sources
 * @n       Some soundfont can be loaded to the ESP32 with PSRAM only
 * @n       Required soundfonts may be downloaded first and stored to the desired memory location (LittleFs, SD-Card)
 */


/*
 * includes
 */
#include "wav_to_sampler.h"
#include "sf_to_sampler.h"


/*
 * extern function definitions
 */
void SoundFontSamplerCtrl(int i)
{
    Serial.printf("SoundFontSamplerCtrl: %d\n", i);
    switch (i)
    {
    case 0:
        /*
         * removing all sample data from sampler
         */
        Sampler_ClearAllSamples();
        break;

    case 1:
        /*
         * load a single sample to all keys
         */
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/PappRohrSample.wav", W2S_ALL_NOTES);
        Sampler_InstrumentDone();
        break;

    case 2:
        /*
         * this is an example loading some wavefiles to different keys
         */
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-01.wav", 36);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-02.wav", 37);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-03.wav", 38);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-04.wav", 39);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-05.wav", 40);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-06.wav", 41);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-07.wav", 42);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-08.wav", 43);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-09.wav", 44);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-10.wav", 45);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-11.wav", 46);
        WavToSmpl_FileToSingleNote(FS_ID_LITTLEFS, "/dirName/fileName-12.wav", 47);
        Sampler_InstrumentDone();
        break;

    case 3:
        /*
         * this example shows how to load all wav files from a folder to different keys
         * starting at a certain note number
         */
        WavToSmpl_FolderToNotes(FS_ID_LITTLEFS, "/dirName", 60);
        break;

    case 4:
        /*
          * this example shows how to load all wav files from a folder to different samples/presets
          * each wav file will be a different voice/sound
          */
        WavToSmpl_FolderToSamples(FS_ID_SD_MMC, "/dirName", 36);
        break;

    case 5:
        /*
         * extracting only single samples including their information from soundfont
         * this can include loop points, but they may be missing because part of instruments
         */
        SF2ToSmpl_LoadAllSamplesFromSF(FS_ID_LITTLEFS, "/Yoshi's Island (WK 1.05).sf2");
        break;

    case 6:
        /*
         * loading all instruments from a soundfont
         * this can include additional information like loop points or override sample information
         * in addition to that this can include different regions for single samples (like a drumset)
         */
        SF2ToSmpl_LoadAllInstrumentsMultiFromSF(FS_ID_SD_MMC, "/Yoshi's Island (WK 1.05).sf2");
        break;

    case 7:
        /*
         * loading all instruments from soundfont
         */
        SF2ToSmpl_LoadAllInstrumentsMultiFromSF(FS_ID_SD_MMC, "/SM64SF V2.sf2");
        break;

    case 8:
        /*
         * loading the complete rhodes soundfont
         * this includes using different samples for different notes and velocity levels
         * it can be loaded completely (requires a bit less than 4MB of PSRAM)
         */
        Serial.printf("Load rhodes samples\n");
        SF2ToSmpl_LoadCompleteSoundFont(FS_ID_SD_MMC, "/198_Rhodes_VS_extreme.sf2");
        break;

    case 9:
        /*
         * another soundfont you could download and try out
         */
        Serial.printf("Load HS Linn Drums.sf2\n");
        SF2ToSmpl_LoadCompleteSoundFont(FS_ID_SD_MMC, "/HS Linn Drums.sf2");
        break;

    case 10:
        /*
         * another soundfont you could download and try out
         */
        Serial.printf("Load Mario samples\n");
        SF2ToSmpl_LoadCompleteSoundFont(FS_ID_LITTLEFS, "/SuperMarioWorld.sf2");
        break;

    case 11:
        /*
         * another soundfont you could download and try out
         * @see https://www.polyphone-soundfonts.com/documents/10-pianos/306-sc55-piano-v2
         */
        Serial.printf("Load SC55 Piano_V2.sf2\n");
        SF2ToSmpl_LoadCompleteSoundFont(FS_ID_SD_MMC, "/SC55 Piano_V2.sf2");
        break;

    case 12:
        /*
         * another soundfont you could download and try out
         */
        Serial.printf("Load HS TR-808 Drums.sf2\n");
        SF2ToSmpl_LoadCompleteSoundFont(FS_ID_SD_MMC, "/HS TR-808 Drums.sf2");
        break;
    }
}
