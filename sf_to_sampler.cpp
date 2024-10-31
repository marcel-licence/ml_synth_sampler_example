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
 * @file sf_to_sampler.cpp
 * @author Marcel Licence
 * @data 09.12.2023
 *
 * @brief   Implementation of function helping to push samples from soundfont into the sampler
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


/*
 * includes
 */
#include <Arduino.h>

#include "config.h"
#include "sf_to_sampler.h"
#include "fs/fs_access.h"

#include <ml_types.h>
#include <ml_status.h>
#include <ml_soundfont.h>
#include <ml_sampler.h>


#define SF2_INFO_MESSAGES

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
static void TransferSampleData(uint32_t start, uint32_t end);
static void LoadAllSamples(void);
static void LoadSampleFromInfo(struct instrLoadInfo_s *info);


/*
 * static function definitions
 */
static void TransferSampleData(uint32_t start, uint32_t end)
{
    Sampler_StartTransfer();
    uint32_t data_to_read = (end - start) * 2;
    fileSeekTo((start) * 2 /* + sampleDataFileOffset */);

    wavSampleS16 sampleData;
    uint32_t bytesRead;

    while (data_to_read > 0)
    {
        if (data_to_read >= 256)
        {
            bytesRead = readBytes(sampleData.data, 256);
        }
        else
        {
            bytesRead = readBytes(sampleData.data, data_to_read);
        }
        if (Sampler_AddSamples(sampleData.samples, bytesRead / 2))
        {
            //Serial.printf("added %d samples, %u left\n", bytesRead, data_to_read);
        }
        else
        {
            Serial.printf("Failed to add %" PRIu32 " samples, %" PRIu32 " left\n", bytesRead, data_to_read);
            break;
        }
        data_to_read -= bytesRead;
    }
    Sampler_EndTransfer();
}

static void LoadAllSamples(void)
{
    struct sf2_soundfont_info_s *offset = ML_SF2_GetSoundFontInfo();

    TransferSampleData(offset->smpl / 2, offset->smpl / 2 + offset->smpl_cnt / 2);

    for (uint32_t i = 0; i < offset->shdr_cnt - 1; i++)
    {
        struct instrLoadInfo_s info;
        if (ML_SF2_LoadSamplesFromInfo(i, &info))
        {
            LoadSampleFromInfo(&info);
            Sampler_InstrumentDone();
        }
    }
}

static void SF2ToSmpl_LoadAllInstrumentsMultiCB(struct instrLoadInfo_s *infoPtr)
{
    Serial.printf("Instrument found: %s\n", infoPtr->name);
    Serial.printf("sample modus: %" PRIu8 "\n", infoPtr->sampleModus);
    Serial.printf("startLoop: %" PRIu32 "\n", infoPtr->startLoop);
    Serial.printf("endLoop: %" PRIu32 "\n", infoPtr->endLoop);
    Serial.printf("start: %" PRIu32 "\n", infoPtr->start);
    Serial.printf("end: %" PRIu32 "\n", infoPtr->end);
    LoadSampleFromInfo(infoPtr);
}

static void LoadSampleFromInfo(struct instrLoadInfo_s *info)
{
    if ((info->start == 0) && (info->end == 0))
    {
        return;
    }

    if (Sampler_NewSample())
    {

        Sampler_NewSampleSetRange(info->start, info->end);

        if ((info->sampleModus == 1) || (info->sampleModus == 3))
        {
            if ((info->startLoop != info->start) || (info->endLoop != info->end))
            {
                info->start += 1;
                printf("loop: %" PRIu32 " - %" PRIu32 "\n", info->startLoop - info->start, info->endLoop - info->start);
                Sampler_NewSampleSetLoop(info->startLoop, info->endLoop);
            }
            Sampler_SetLoopMode(info->sampleModus);
        }

        Sampler_SetExclusiveClass(info->exClass);
        Sampler_SetPitch(info->rootKey, info->sampleRate, info->tune);
#ifdef MULTIPLE_SAMPLE_PER_INSTRUMENT
        Sampler_SetKeyRange(info->keyRange.lowest, info->keyRange.highest);
        Sampler_SetVelRange(info->velRange.lowest, info->velRange.highest);
#endif

        {
            float holdVolEnv_f = info->decayVolEnv;
            holdVolEnv_f /= 1200;
            holdVolEnv_f = pow(2, holdVolEnv_f);

            /* time normalized by sample rate */
            holdVolEnv_f *= ((float)SAMPLE_RATE);

            /* multiplication count to min 16 bit value */
            holdVolEnv_f = pow(1.0f / 32736.0f, 1.0 / holdVolEnv_f);

            //pow(1.0/32736.0, 1/(holdVolEnv_f*SAMPLE_RATE));

            /* normalize */
            holdVolEnv_f *= 2147483648.0f;

            Sampler_SetHold((uint32_t)holdVolEnv_f);
        }

        {
            float releaseVolEnv_f = info->releaseVolEnv;
            releaseVolEnv_f /= 1200;
            releaseVolEnv_f = pow(2, releaseVolEnv_f);

            /* time normalized by sample rate */
            releaseVolEnv_f *= ((float)SAMPLE_RATE);

            /* multiplication count to min 16 bit value */
            releaseVolEnv_f = pow(1.0f / 32736.0f, 1.0 / releaseVolEnv_f);

            //pow(1.0/32736.0, 1/(releaseVolEnv_f*SAMPLE_RATE));

            /* normalize */
            releaseVolEnv_f *= 2147483648.0f;

            Sampler_SetRelease((uint32_t)releaseVolEnv_f);
        }


        Sampler_FinishSample();
    }
    else
    {
        Serial.printf("Could not add sample (LoadSampleFromInfo3)!\n");
    }
}


/*
 * extern function definitions
 */
void SF2ToSmpl_LoadAllInstrumentsMulti(void)
{
    struct sf2_soundfont_info_s *offset = ML_SF2_GetSoundFontInfo();

    TransferSampleData(offset->smpl / 2, offset->smpl / 2 + offset->smpl_cnt / 2);

    for (uint32_t i = 0; i < offset->inst_cnt - 1; i++)
    {

        if (ML_SF2_GetInstrumentInfoMultiBag(i, SF2ToSmpl_LoadAllInstrumentsMultiCB))
        {
            Sampler_InstrumentDone();
        }
        else
        {
            Serial.printf("loadInstr failed!\n");
        }
    }
}

void SF2ToSmpl_LoadAllInstruments(void)
{
    struct sf2_soundfont_info_s *offset = ML_SF2_GetSoundFontInfo();

    TransferSampleData(offset->smpl / 2, offset->smpl / 2 + offset->smpl_cnt / 2);

    for (uint32_t i = 0; i < offset->inst_cnt - 1; i++)
    {
        struct instrLoadInfo_s info;
        if (ML_SF2_GetInstrumentInfo(i, &info))
        {
            LoadSampleFromInfo(&info);
            Sampler_InstrumentDone();
        }
        else
        {
            Serial.printf("loadInstr failed!\n");
        }
    }
}

void SF2ToSmpl_LoadCompleteSoundFont(void)
{
    struct sf2_soundfont_info_s *offset = ML_SF2_GetSoundFontInfo();

    TransferSampleData(offset->smpl / 2, offset->smpl / 2 + offset->smpl_cnt / 2);

    for (uint32_t i = 0; i < offset->phdr_cnt - 1; i++)
    {
        if (ML_SF2_LoadPresetMultiBag(i, LoadSampleFromInfo))
        {
            Sampler_InstrumentDone();
        }
        else
        {
            Serial.printf("loadPresetMultiBag failed!\n");
        }
    }
}

void SF2ToSmpl_LoadAllInstrumentsFromSF(fs_id_t fs_id, const char *filename)
{
    if (FS_OpenFile(fs_id, filename))
    {
        SF2ToSmpl_LoadAllInstruments();
        FS_CloseFile();

        Status_ValueChangedStr("Instruments from Soundfont", "Loaded", filename);
    }
    else
    {
        Status_ValueChangedStr("Instruments from Soundfont", "Loading failed!", filename);
    }
}

void SF2ToSmpl_LoadAllInstrumentsMultiFromSF(fs_id_t fs_id, const char *filename)
{
    if (FS_OpenFile(fs_id, filename))
    {
        SF2ToSmpl_LoadAllInstrumentsMulti();
        FS_CloseFile();

        Status_ValueChangedStr("Instruments from Soundfont", "Loaded", filename);
    }
    else
    {
        Status_ValueChangedStr("Instruments from Soundfont", "Loading failed!", filename);
    }
}

void SF2ToSmpl_LoadCompleteSoundFont(fs_id_t fs_id, const char *filename)
{
    if (FS_OpenFile(fs_id, filename))
    {
        SF2ToSmpl_LoadCompleteSoundFont();
        FS_CloseFile();

        Status_ValueChangedStr("Complete Soundfont", "Loaded", filename);
    }
    else
    {
        Status_ValueChangedStr("Complete Soundfont", "Loading failed!", filename);
    }
}

void SF2ToSmpl_LoadAllSamplesFromSF(fs_id_t fs_id, const char *filename)
{
    if (FS_OpenFile(fs_id, filename))
    {
        LoadAllSamples();
        FS_CloseFile();

        Status_ValueChangedStr("Samples from Soundfont", "Loaded", filename);
    }
    else
    {
        Status_ValueChangedStr("Samples from Soundfont", "Loading failed!", filename);
    }
}

/**
 *  @brief Handler for soundfont preset indications
 */
void sf2_preset_indication(union preset_hdr_s *preset, uint32_t idx)
{
#ifdef SF2_INFO_MESSAGES
    Serial.printf("presetName[%" PRIu32 "]: %s\n", idx, preset->presetName);
    Serial.printf("  preset: %d\n", preset->preset);
    Serial.printf("  bank: %d\n", preset->bank);
    Serial.printf("  presetBagIndex: %d\n", preset->presetBagIndex);
    Serial.printf("  library: %" PRIu32 "\n", preset->library);
    Serial.printf("  genre: %" PRIu32 "\n", preset->genre);
    Serial.printf("  morphology: %" PRIu32 "\n", preset->morphology);
#else
    (void)preset;
    (void)idx;
#endif
}

/**
 *  @brief Handler for soundfont sample indications
 */
void sf2_sample_indication(union sf2_sample_hdr_s *sample, uint32_t idx)
{
#ifdef SF2_INFO_MESSAGES
    char sampleName[21] = {0};
    memcpy(sampleName, sample->sampleName, 20);
    Serial.printf("sampleName[%" PRIu32 "]: %s\n", idx, sampleName);
    Serial.printf("  start: %" PRIu32 "\n", sample->start);
    Serial.printf("  end: %" PRIu32 "\n", sample->end);
    Serial.printf("  startLoop: %" PRIu32 "\n", sample->startLoop);
    Serial.printf("  endLoop: %" PRIu32 "\n", sample->endLoop);
    Serial.printf("  sampleRate: %" PRIu32 "\n", sample->sampleRate);
    Serial.printf("  originalPitch: %d\n", sample->originalPitch);
    Serial.printf("  pitchCorrection: %d\n", sample->pitchCorrection);
    Serial.printf("  sampleLink: %d\n", sample->sampleLink);
    Serial.printf("  sampleType: %d\n", sample->sampleType);
#else
    (void)sample;
    (void)idx;
#endif
}

/**
 * @brief Will be called when a soundfont will be parsed and a sample will be identified
 *
 * @param inst
 * @param idx
 */
void sf2_instrument_indication(union SF2Instrument_u *inst, uint32_t idx)
{
#ifdef SF2_INFO_MESSAGES
    char instName[21] = {0};
    strncpy(instName, inst->name, 20);
    Serial.printf("instrument[%" PRIu32 "]:\n", idx);
    Serial.printf("  instName: %s\n", instName);
    Serial.printf("  bagIndex: %u\n", inst->bagIndex);
#else
    (void)inst;
    (void)idx;
#endif
}

/**
 *  @brief Handler for soundfont sample tag indication
 */
void sf2_sdta_smpl_indication(uint32_t len)
{
#ifdef SF2_INFO_MESSAGES
    Serial.printf("Sample in file at %" PRIu32 "\n", getStaticPos());
    Serial.printf("    len %" PRIu32 "\n", len);
#else
    (void)len;
#endif
}

void sf2_preset_bag_indication(union SF2PresetBag_u *pbag)
{
#ifdef SF2_INFO_MESSAGES
    Serial.printf("preset bag:\n");
    Serial.printf("  generatorIndex: %u\n", pbag->generatorIndex);
    Serial.printf("  modulatorIndex: %u\n", pbag->modulatorIndex);
#else
    (void)pbag;
#endif
}

void sf2_preset_modulator_indication(union SF2PresetModulator_u *pmod)
{
#ifdef SF2_INFO_MESSAGES
    Serial.printf("preset modulator:\n");
    Serial.printf("  sourceOperator: %u\n", pmod->sourceOperator);
    Serial.printf("  destinationOperator: %u\n", pmod->destinationOperator);
    Serial.printf("  amount: %d\n", pmod->amount);
    Serial.printf("  amountSourceOperator: %u\n", pmod->amountSourceOperator);
    Serial.printf("  transportOperator: %u\n", pmod->transportOperator);
#else
    (void)pmod;
#endif
}

void sf2_preset_generator_indication(union SF2PresetGenerator_u *pgen, uint32_t idx)
{
#ifdef SF2_INFO_MESSAGES
    Serial.printf("preset generator[%" PRIu32 "]:\n", idx);
    Serial.printf("  generatorIndex: %u\n", pgen->generatorIndex);
    Serial.printf("  amount: %d\n", pgen->amount);
#else
    (void)pgen;
    (void)idx;
#endif
}

void sf2_instrument_bag_indication(union SF2InstrumentBag_u *ibag, uint32_t idx)
{
#ifdef SF2_INFO_MESSAGES
    Serial.printf("instrument bag[%" PRIu32 "]:\n", idx);
    Serial.printf("  generatorIndex: %u\n", ibag->generatorIndex);
    Serial.printf("  modulatorIndex: %u\n", ibag->modulatorIndex);
#else
    (void)ibag;
    (void)idx;
#endif
}

void sf2_instrument_generator_indication(union SF2InstrumentGenerator_u *igen, uint32_t idx)
{
#ifdef SF2_INFO_MESSAGES
    Serial.printf("instrument generator[%" PRIu32 "]:\n", idx);
    Serial.printf("  ioperator: %u\n", igen->ioperator);
    Serial.printf("  amount: %u\n", igen->amount);
#else
    (void)igen;
    (void)idx;
#endif
}

