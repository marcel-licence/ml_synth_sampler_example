/*
 * Copyright (c) 2024 Marcel Licence
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
 * @file wav_to_sampler.cpp
 * @author Marcel Licence
 * @data 05.12.2023
 *
 * @brief   This file contains some functions to transfer wav-files to the sampler
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


/*
 * includes
 */
#include <Arduino.h>

#include <ml_sampler.h>
#include <ml_status.h>

#include "fs/fs_access.h"
#include "utils.h"
#include "ml_wavfile.h"
#include "wav_to_sampler.h"


/*
 * data types
 */
union wavSampleS16
{
    uint8_t data[256];
    Q1_14 samples[128];
};


/*
 * static function declarations
 */
static void wavToSmpl_ReadWaveFile(const char *filename, uint8_t note);
static bool wavToSmpl_WavData(union wavHeader *hdr, uint16_t bytesPerSample, uint32_t data_to_read, uint8_t note);
static void wavToSmpl_FolderToNotes_CB(const char *filename, int depth __attribute__((unused)), uint8_t note);
static void wavToSmpl_FolderToSamples_CB(const char *filename, int depth __attribute__((unused)), uint8_t note);


/*
 * static function definitions
 */
static void wavToSmpl_FolderToNotes_CB(const char *filename, int depth __attribute__((unused)), uint8_t note)
{
    if (str_ends_with(filename, ".wav"))
    {
        Serial.printf("Wavefile detected (note: %u)...\n", note);
        FS_UseTempFile();
        wavToSmpl_ReadWaveFile(filename, note);
    }
}

static void wavToSmpl_FolderToSamples_CB(const char *filename, int depth __attribute__((unused)), uint8_t note)
{

    if (str_ends_with(filename, ".wav"))
    {
        Serial.printf("Wavefile detected (note: %u)...\n", note);
        FS_UseTempFile();
        wavToSmpl_ReadWaveFile(filename, 255);
    }
    //Sampler_InstrumentDone();
}

static bool wavToSmpl_WavData(union wavHeader *hdr, uint16_t bytesPerSample, uint32_t data_to_read, uint8_t note)
{
    if (Sampler_NewSample())
    {
        Sampler_StartTransfer();
        while (data_to_read >= bytesPerSample)
        {
            wavSampleS16 sampleData;
            uint32_t bytesRead = 0;
            uint32_t nextBlock = data_to_read > sizeof(sampleData) ? sizeof(sampleData) : data_to_read;
            uint32_t samplesInBlock = 0;

#if 1
            if (hdr->numberOfChannels == 2)
            {
                wavSampleS16 sampleDataTemp;

                bytesRead = readBytes(sampleDataTemp.data, nextBlock);
                if (bytesRead != nextBlock)
                {
                    /* error occurred */
                    Serial.printf("readError %" PRIu32 "", bytesRead);
                    return false;
                }

                samplesInBlock = bytesRead / 4;

                for (uint32_t i = 0; i < samplesInBlock; i++)
                {
                    sampleData.samples[i] = sampleDataTemp.samples[2 * i];
                }


            }
#endif
            if (hdr->numberOfChannels == 1)
            {
                bytesRead = readBytes(sampleData.data, nextBlock);
                if (bytesRead != nextBlock)
                {
                    /* error occurred */
                    Serial.printf("readError %" PRIu32 "", bytesRead);
                    return false;
                }
                samplesInBlock = bytesRead / bytesPerSample;
            }
            // Serial.printf("%u, %u x %u\n", nextBlock, data_to_read, samplesInBlock);


            if (bytesPerSample == 1)
            {
                if (Sampler_AddSamplesU8(sampleData.data, samplesInBlock) == false)
                {
                    Serial.printf("Failed to add %" PRIu32 " samples, %" PRIu32 " left\n", bytesRead, data_to_read);
                    break;
                }
            }
            else
            {
                if (Sampler_AddSamples(sampleData.samples, samplesInBlock) == false)
                {
                    Serial.printf("Failed to add %" PRIu32 " samples, %" PRIu32 " left\n", bytesRead, data_to_read);
                    break;
                }
            }
            data_to_read -= bytesRead;
        }
        Sampler_EndTransfer();

        Sampler_NewSampleSetRange(0, hdr->nextTag.tag_data_size / hdr->bytesPerSample - 1); /* expecting 16 bit */


        if (note != 0xFF)
        {
            Sampler_SetKeyRange(note, note);
            Sampler_SetPitch(note, hdr->sampleRate, 0);
        }
        else
        {
            Sampler_SetPitch(60, hdr->sampleRate, -82);
        }

        return true;

    }
    else
    {
        Serial.printf("unable to add samples\n");
        return false;
    }
}

static void wavToSmpl_ReadWaveFile(const char *filename, uint8_t note)
{
    Serial.printf("Reading wav: %s\n", filename);
    union wavHeader wavHdr;
    int bytesRead = readBytes(wavHdr.wavHdr, sizeof(wavHdr.wavHdr));
    if (bytesRead == sizeof(wavHdr.wavHdr))
    {
        Serial.printf("  fileSize: %" PRIu32 "\n", wavHdr.fileSize);
        Serial.printf("  lengthOfData: %" PRIu32 "\n", wavHdr.lengthOfData);
        Serial.printf("  byteRate: %" PRIu32 "\n", wavHdr.byteRate);
        Serial.printf("  bytesPerSample: %" PRIu16 "\n", wavHdr.bytesPerSample);
        Serial.printf("  bitsPerSample: %" PRIu16 "\n", wavHdr.bitsPerSample);
        Serial.printf("  sampleRate: %" PRIu32 "\n", wavHdr.sampleRate);
        Serial.printf("  numberOfChannels: %" PRIu16 "\n", wavHdr.numberOfChannels);
    }
    else
    {
        Serial.printf("error reading wave header!\n");
    }

    if (wavHdr.lengthOfData > 16)
    {
        fileSeekTo(getCurrentOffset() + wavHdr.lengthOfData - 16);
    }

    while (memcmp(wavHdr.nextTag.tag_name, "data", 4) != 0)
    {
        char tagStr[5] = {0};
        memcpy(tagStr, wavHdr.nextTag.tag_name, 4);
        Serial.printf("skip tag: %s\n", tagStr);


        fileSeekTo(getCurrentOffset() + wavHdr.nextTag.tag_data_size);

        int bytesRead = readBytes(wavHdr.nextTag.wavHdr, sizeof(wavHdr.nextTag.wavHdr));
        if (bytesRead != sizeof(wavHdr.nextTag.wavHdr))
        {
            Serial.printf("error reading tag\n");
            return;
        }
    }

    bool wavAdded = false;

    if (memcmp(wavHdr.nextTag.tag_name, "data", 4) == 0)
    {
        Serial.printf("data found\n");

        uint32_t data_to_read = wavHdr.nextTag.tag_data_size;

        wavAdded = wavToSmpl_WavData(&wavHdr, wavHdr.bytesPerSample, data_to_read, note);
    }

    bytesRead = readBytes(wavHdr.nextTag.wavHdr, sizeof(wavHdr.nextTag.wavHdr));
    if (bytesRead != sizeof(wavHdr.nextTag.wavHdr))
    {
        Serial.printf("error reading tag\n");
    }

    if (memcmp(wavHdr.nextTag.tag_name, "smpl", 4) == 0)
    {
        union wav_tag__smpl_u smpl_tag;

        int bytesRead = readBytes(smpl_tag.raw, sizeof(smpl_tag.raw));
        if (bytesRead != sizeof(smpl_tag.raw))
        {
            Serial.printf("error occured reading smpl data\n");
            return;
        }
        Serial.printf("SMPL tag detected:\n");
        Serial.printf("  manufacturer: %" PRIu32 "\n", smpl_tag.manufacturer);
        Serial.printf("  product: %" PRIu32 "\n", smpl_tag.product);
        Serial.printf("  sample_period: %" PRIu32 "\n", smpl_tag.sample_period);
        Serial.printf("  MIDI_unity_note: %" PRIu32 "\n", smpl_tag.MIDI_unity_note);
        Serial.printf("  MIDI_pitch_fraction: %" PRIu32 "\n", smpl_tag.MIDI_pitch_fraction);
        Serial.printf("  SMPTE_format: %" PRIu32 "\n", smpl_tag.SMPTE_format);
        Serial.printf("  SMPTE_offset: %" PRIu32 "\n", smpl_tag.SMPTE_offset);
        Serial.printf("  number_of_sample_loops: %" PRIu32 "\n", smpl_tag.number_of_sample_loops);
        Serial.printf("  sample_data: %" PRIu32 "\n", smpl_tag.sample_data);
        Serial.printf("  sample loops:\n");
        Serial.printf("    ID: %" PRIu32 "\n", smpl_tag.ID);
        Serial.printf("    type: %" PRIu32 "\n", smpl_tag.type);
        Serial.printf("    start: %" PRIu32 "\n", smpl_tag.start);
        Serial.printf("    end: %" PRIu32 "\n", smpl_tag.end);
        Serial.printf("    fraction: %" PRIu32 "\n", smpl_tag.fraction);
        Serial.printf("    number_of_times_to_play_the_loop: %" PRIu32 "\n", smpl_tag.number_of_times_to_play_the_loop);

        Sampler_SetPitch(smpl_tag.MIDI_unity_note, wavHdr.sampleRate, 0);
        Sampler_NewSampleSetLoop(smpl_tag.start, smpl_tag.end);
        Sampler_SetLoopMode(1);
    }

    if (wavAdded)
    {
        Sampler_FinishSample();
        if (note == W2S_ALL_NOTES)
        {
            Sampler_InstrumentDone();
        }
    }

#ifdef SAMPLER_DYNAMIC_BUFFER_SIZE
    int samples = readBytes((uint8_t *)&sample_buffer[sample_info[sample_cnt].first], sizeof(sample_buffer));
    samples /= 2;
    sample_info[sample_cnt].last = sample_info[sample_cnt].first + samples;
#endif
}

/*
 * extern function definitions
 */
void WavToSmpl_FolderToNotes(fs_id_t id, const char *dirname, uint8_t start_note)
{
    WavToKeyboard(id, dirname, wavToSmpl_FolderToNotes_CB, 0, 10, start_note);
    Sampler_InstrumentDone();
    Status_ValueChangedStr("Wav Files from Dir", "Loaded to notes", dirname);
}

void WavToSmpl_FolderToSamples(fs_id_t id, const char *dirname, uint8_t start_note)
{
    WavToKeyboard(id, dirname, wavToSmpl_FolderToSamples_CB, 0, 10, start_note);
    Status_ValueChangedStr("Wav Files from Dir", "Loaded to samples", dirname);
}

void WavToSmpl_FileToSingleNote(fs_id_t id, const char *filename, uint8_t note)
{
    if (FS_OpenFile(id, filename))
    {
        wavToSmpl_ReadWaveFile(filename, note);
        FS_CloseFile();
    }
}

