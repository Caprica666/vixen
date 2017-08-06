#pragma once
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

class VXSoundData 
{
public:
	VXSoundData();
	VXSoundData(const char *);
	~VXSoundData();

	// Wave file operations
	ALvoid		LoadWAVFile(const char* = NULL);
	ALvoid		LoadWAVMemory();
	ALvoid		UnloadWAV();
	ALenum*		GetFormat() const {return m_format; }
	ALvoid*		GetData() const { return m_data; }
	ALsizei*	GetSize() const { return m_size; }
	ALsizei*	GetFrequency() const { return m_freq; }
	ALfloat		GetTime() { return (*m_size)/m_FmtHdr.BytesPerSec; } 
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