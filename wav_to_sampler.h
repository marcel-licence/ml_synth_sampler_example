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
 * @file wav_to_sampler.h
 * @author Marcel Licence
 * @data 05.12.2023
 *
 * @brief   This file contains the prototypes of function to transfer wav-files to the sampler
 */


#ifndef WAV_TO_SAMPLER_H_
#define WAV_TO_SAMPLER_H_


/*
 * includes
 */
#include "fs/fs_access.h"


/*
 * defines
 */
#define W2S_ALL_NOTES   0xFF


/*
 * declarations
 */
void WavToSmpl_FileToSingleNote(fs_id_t id, const char *filename, uint8_t note);
void WavToSmpl_FolderToSamples(fs_id_t id, const char *dirname, uint8_t start_note);
void WavToSmpl_FolderToNotes(fs_id_t id, const char *dirname, uint8_t start_note);


#endif /* WAV_TO_SAMPLER_H_ */
