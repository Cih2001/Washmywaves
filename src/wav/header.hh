#ifndef WASHMYWAVES_WAV_HEADER_H__
#define WASHMYWAVES_WAV_HEADER_H__

#include <cstdint>
#include <istream>
#include <string>
#include <memory>

#define WAVE_FORMAT_PCM        0x0001 
#define WAVE_FORMAT_IEEE_FLOAT 0x0003 
#define WAVE_FORMAT_ALAW       0x0006 
#define WAVE_FORMAT_MULAW      0x0007 
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE 

// WavHeader is a class to extract information from wav file formats.
class WavHeader {
public:
  // wav file format related structs are specified in:
  // http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
  struct ChunkHeader {
    uint32_t id;
    uint32_t size;
  };

  struct RiffChunk {
    ChunkHeader chunk_header;
    uint32_t format;
  };

  struct FmtChunk {
    ChunkHeader chunk_header;
    uint16_t format_tag;
    uint16_t number_of_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    // the rest of struct mwmbers are only present in extensible wav format.
    uint16_t valid_bits_per_sample;
    uint32_t channel_mask;
    union {
      uint16_t audio_format;
      uint8_t sub_format_guid[16];
    };
  };

  struct DataChunk {
    ChunkHeader chunk_header;
    uint8_t data[0];
  };

  WavHeader(std::istream& input);

  // @desc - checks if the input stream is a valid wav file.
  // @return bool - true is a valid wav file is opened.
  bool IsValidWav();

  // @desc - returns format chuck header of wav files.
  // @return FmtChunk
  FmtChunk GetFormatChunkHeader();

  // @desc - used to determine data size in data chuck.
  // return size_t - size of data.
  size_t GetDataSize();

  // @desc - used to determine number of samples in data chuck.
  // return size_t - number of samples.
  size_t GetNumberOfSamples();

  // @desc - returns the index of raw data in the input stream. 
  // return int - index of raw data, on error 0.
  int GetDataIndex();

  // @desc - returns the the format of audio data.
  // return uint16_t - can be WAVE_FORMAT_PCM, WAVE_FORMAT_...
  uint16_t GetAudioFormat();

  // @desc - returns the aplitude-scaled pcm data.  
  // @param channel - can be 0 (for left channel) or 1 (for right channel).
  // @return std::unique_ptr<std::string> - a buffer containing pcm data.
  std::unique_ptr<std::string> ReadPCMData(unsigned int channel);

private:
  std::istream& input_;

  // @desc - finds the index of fmt chunk header.
  // @return int - index of fmt chunk header or -1 on error. 
  int FindFormatChunkHeader();

  // @desc - finds the index of data chunk header.
  // @return int - index of data chunk header or -1 on error. 
  int FindDataChunkHeader();
};

#endif // WASHMYWAVES_WAV_HEADER_H__
