#ifndef WASHMYWAVES_WAV_HEADER_H__
#define WASHMYWAVES_WAV_HEADER_H__

#include <cstdint>
#include <istream>

class WavHeader {
public:
  WavHeader(std::istream& input);
  
  bool IsValidWav();

private:
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
    uint16_t audio_format;
    uint16_t num_of_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
  };

  struct DataChunk {
    ChunkHeader chunk_header;
    uint8_t data[0];
  };

  std::istream& input_;

};

#endif // WASHMYWAVES_WAV_HEADER_H__
