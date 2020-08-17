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

class WavHeader {
public:
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
  bool IsValidWav();
  FmtChunk GetFormatChunkHeader();
  size_t GetDataSize();
  size_t GetNumberOfSamples();
  int GetDataIndex();
  uint16_t GetAudioFormat();

  std::unique_ptr<std::string> ReadPCMData(unsigned int channel);

private:
  std::istream& input_;

  int FindFormatChunkHeader();
  int FindDataChunkHeader();
};

#endif // WASHMYWAVES_WAV_HEADER_H__
