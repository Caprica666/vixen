/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */
#ifndef _PS_SOUNDDATA
#define _PS_SOUNDDATA

#include "altypes.h"

#pragma pack (push,1) 							/* Turn off alignment */

typedef struct                                  /* WAV File-header */
{
  ALubyte  Id[4];
  ALsizei  Size;
  ALubyte  Type[4];
} WAVFileHdr_Struct;

typedef struct                                  /* WAV Fmt-header */
{
  ALushort Format;                              
  ALushort Channels;
  ALuint   SamplesPerSec;
  ALuint   BytesPerSec;
  ALushort BlockAlign;
  ALushort BitsPerSample;
} WAVFmtHdr_Struct;

typedef struct									/* WAV FmtEx-header */
{
  ALushort Size;
  ALushort SamplesPerBlock;
} WAVFmtExHdr_Struct;

typedef struct                                  /* WAV Smpl-header */
{
  ALuint   Manufacturer;
  ALuint   Product;
  ALuint   SamplePeriod;                          
  ALuint   Note;                                  
  ALuint   FineTune;                              
  ALuint   SMPTEFormat;
  ALuint   SMPTEOffest;
  ALuint   Loops;
  ALuint   SamplerData;
  struct
  {
    ALuint Identifier;
    ALuint Type;
    ALuint Start;
    ALuint End;
    ALuint Fraction;
    ALuint Count;
  }      Loop[1];
} WAVSmplHdr_Struct;

typedef struct                                  /* WAV Chunk-header */
{
  ALubyte  Id[4];
  ALuint   Size;
} WAVChunkHdr_Struct;

#pragma pack (pop)								/* Default alignment */

class PSSoundData 
{
public:
	PSSoundData();
	PSSoundData(const char *);
	~PSSoundData();

	// Wave file operations
	ALvoid		LoadWAVFile(const char* = NULL);
	ALvoid		LoadWAVMemory();
	ALvoid		UnloadWAV();
	ALenum*		GetFormat() const {return m_format; }
	ALvoid*		GetData() const { return m_data; }
	ALsizei*	GetSize() const { return m_size; }
	ALsizei*	GetFrequency() const { return m_freq; }
	ALfloat		GetTime() { return (ALfloat) (*m_size)/m_FmtHdr.BytesPerSec; } 
	ALsizei		GetChannels() { return m_FmtHdr.Channels; }
	ALsizei		GetBitsPerSample() { return m_FmtHdr.BitsPerSample; }
	ALsizei		GetBlockAlign() { return m_FmtHdr.BlockAlign; }
	ALsizei		GetBytesPerSecond() { return m_FmtHdr.BytesPerSec; }

protected:
	WAVFileHdr_Struct			m_FileHdr;
	WAVFmtHdr_Struct			m_FmtHdr;
	WAVFmtExHdr_Struct			m_FmtExHdr;
	WAVSmplHdr_Struct			m_SmplHdr;
	WAVChunkHdr_Struct			m_ChunkHdr;

	ALbyte						*m_memory;
	ALenum						*m_format;
	ALvoid						*m_data;
	ALsizei						*m_size;
	ALsizei						*m_freq;
	ALbyte						*m_file;

private:
};
#endif;