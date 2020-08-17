#include <algorithm>
#include <iterator>
#include <cstring>
#include <cmath>

#include "wav/header.hh"
#include "utils/global.hh"

#define RIFF_CHUNK_ID 0x46464952
#define RIFF_FORMAT_WAVE 0x45564157
#define FMT_CHUNK_ID 0x20746d66
#define DATA_CHUNK_ID 0x61746164

WavHeader::WavHeader(std::istream& input) : input_(input) {
  if (!input_) throw std::runtime_error("cannot open input stream.");
}

template<typename T>
T CastBytes(std::istream& stream) {
  char bytes[sizeof(T)];
  stream.read(bytes, sizeof(T));
  return *(T*)(bytes);
}

bool WavHeader::IsValidWav() {
  input_.seekg(0);

  auto riff_header = CastBytes<RiffChunk>(input_);
  if (!input_) {
    // stream not long enough or an internal error in stream.
    return false;
  }
  HEX_DUMP((const unsigned char *)&riff_header, sizeof(riff_header));
  if (riff_header.chunk_header.id != RIFF_CHUNK_ID ||
      riff_header.format != RIFF_FORMAT_WAVE) {
    return false;
  }
  auto fmt_pos = FindFormatChunkHeader();
  if (fmt_pos < 0) {
    return false;
  }

  // TODO: we need more checks, like presence of data, chunks size and etc.
  return true;
}

int WavHeader::FindFormatChunkHeader() {
  input_.seekg(sizeof(RiffChunk));

  do {
    auto chunk = CastBytes<ChunkHeader>(input_);
    if (!input_) {
      // stream not long enough or an internal error in stream.
      return -1;
    }
    if (chunk.id == FMT_CHUNK_ID) {
      return (int)input_.tellg() - sizeof(chunk);
    }
    input_.seekg((int)input_.tellg() + chunk.size);
  } while (true);
  return -1;
}

WavHeader::FmtChunk WavHeader::GetFormatChunkHeader() {
  WavHeader::FmtChunk result;
  auto fmt_pos = FindFormatChunkHeader();
  if (fmt_pos >= 0) {
    input_.seekg(fmt_pos);
    result = CastBytes<FmtChunk>(input_);
  }
  return result;
}

int WavHeader::FindDataChunkHeader() {
  input_.seekg(sizeof(RiffChunk));
  do {
    auto chunk = CastBytes<ChunkHeader>(input_);
    if (!input_) {
      // stream not long enough or an internal error in stream.
      return -1;
    }

    if (chunk.id == DATA_CHUNK_ID) {
      return (int)input_.tellg() - sizeof(chunk);
    }
    input_.seekg((int)input_.tellg() + chunk.size);
  } while (true);
  return -1;
}

size_t WavHeader::GetDataSize() {
  auto data_pos = FindDataChunkHeader();
  if (data_pos >= 0) {
    input_.seekg(data_pos);
    auto result = CastBytes<DataChunk>(input_);
    return result.chunk_header.size;
  }
  return 0;
}

size_t WavHeader::GetNumberOfSamples() {
  auto block_align = GetFormatChunkHeader().block_align;
  return GetDataSize() / block_align;
}

int WavHeader::GetDataIndex() {
  auto data_pos = FindDataChunkHeader();
  if (data_pos >= 0) {
    return data_pos + offsetof(DataChunk, data);
  }
  return 0;
}

uint16_t map16(uint16_t val, unsigned int val_bits) {
  if (val == 0) return 0;
  int16_t out_minimum = -0x7fff - 1; 
  int16_t out_maximum = 0x7fff; 
  int16_t in_minimum = - (1 << (val_bits - 1));
  int16_t in_maximum = (1 << (val_bits - 1)) - 1;

  double pos = (double)(val - in_minimum) / (double)( in_maximum - in_minimum);
  auto result = ((double)out_maximum - (double)out_minimum) * pos +
      (double)out_minimum;
  return (uint16_t) result;
}

uint32_t map32(uint32_t val, unsigned int val_bits) {
  if (val == 0) return 0;
  const int32_t out_minimum = -0x7fffffff - 1;
  const int32_t out_maximum = 0x7fffffff;
  int32_t in_minimum = - (1 << (val_bits - 1));
  int32_t in_maximum = (1 << (val_bits - 1)) - 1;

  double pos = (double)(val - in_minimum) / (double)( in_maximum - in_minimum);
  auto result = ((double)out_maximum - (double)out_minimum) * pos +
      (double)out_minimum;
  return (uint32_t) result;
}

uint16_t ScaleAmplitude8(uint8_t val) {
  // 8-bit PCM is unsigned with the center point of 128.
  return map16(val - 128, 8);
}

uint16_t ScaleAmplitude16(uint16_t val, unsigned int bits_per_sample) {
  unsigned int sample_bits_ceiling = ceil((float)bits_per_sample / 8) * 8;
  // clear garbage bits in the padded bytes.
  if (bits_per_sample < 16) {
    val &= (1 << bits_per_sample) - 1; 
  }
  uint16_t out_val = 0;
  for (unsigned int b = 0; b < bits_per_sample; b++) {
    out_val |= (val & (1 << b)) ? 1 << b : 0;
  }
  // convert value to 2nd compliment value.
  if ((bits_per_sample < sample_bits_ceiling) &&
      ((out_val & (1 << (bits_per_sample - 1))) != 0)) {
    out_val -= 1 << (bits_per_sample);
  }
  return map16(out_val, bits_per_sample);
}

uint32_t ScaleAmplitude32(uint32_t val, unsigned int bits_per_sample) {
  unsigned int sample_bits_ceiling = ceil((float)bits_per_sample / 8) * 8;
  // clear garbage bits in the padded bytes.
  if (bits_per_sample < 32) {
    val &= (1 << bits_per_sample) - 1; 
  }
  uint32_t out_val = 0;
  for (unsigned int b = 0; b < bits_per_sample; b++) {
    out_val |= (val & (1 << b)) ? 1 << b : 0;
  }
  // convert value to 2nd compliment value.
  if ((bits_per_sample < sample_bits_ceiling) &&
      ((out_val & (1 << (bits_per_sample - 1))) != 0)) {
    out_val -= 1 << (bits_per_sample);
  }
  return map32(out_val, bits_per_sample);
}

uint16_t WavHeader::GetAudioFormat() {
  auto fmt_chunk = GetFormatChunkHeader();
  if (fmt_chunk.format_tag != WAVE_FORMAT_EXTENSIBLE) {
    return fmt_chunk.format_tag;
  }
  return  fmt_chunk.audio_format;
}

std::unique_ptr<std::string> WavHeader::ReadPCMData(unsigned int channel) {
  auto data_index = GetDataIndex();
  auto number_of_samples = GetNumberOfSamples();
  auto block_align = GetFormatChunkHeader().block_align;
  auto bits_per_sample = GetFormatChunkHeader().bits_per_sample;

  // bits-width in some wav file are not divisable by 8, like 12, 20 and ...
  // the cases, although uncommon, should be scaled up to 8 bits divided width.
  // for example, to store a 12-bits sample, 16-bits are needed and 24-bits,
  // for 20-bits sample.
  unsigned int sample_bits_ceiling = ceil((float)bits_per_sample / 8) * 8;

  auto number_of_channels = GetFormatChunkHeader().number_of_channels;
  auto result = new std::string();
  if (number_of_channels == 1 && channel > 0) {
    // unsupported format.
    return std::unique_ptr<std::string>(result);
  }

  // alocating space for the output buffer.
  // we use std::string as an output buffer, because it is really easy to
  // resize it and thus allocate buffer, without being worry of memory leaks.
  auto buffer_size = number_of_samples;
  if (sample_bits_ceiling <= 16) {
    // each sample will be stored in a short int.
    buffer_size *= 2;
  } else {
    // each sample will be stored in an int.
    buffer_size *= 4;
  }
  result->resize(buffer_size);

  // read pcm data.
  input_.seekg(data_index);
  for (size_t i = 0; i < number_of_samples; i++) {
    // read one block.
    // each block contains a sample for each channel which are interleaved.
    char block[block_align];
    std::memset(block, 0, block_align);
    input_.read(block, block_align);
    // if (i < 5) {
    //   HEX_DUMP((const unsigned char *)block, 8);
    // }
    if (sample_bits_ceiling == 8) {
      // 8-bits width samples can be read into a byte.
      uint8_t unscaled_value = ((uint8_t *) block)[(channel == 0) ? 0 : 1];
      auto out_offset = &((uint16_t *) result->c_str())[i];
      *out_offset = ScaleAmplitude8(unscaled_value);
    }
    else if (sample_bits_ceiling == 16) {
      // 16-bits width samples can be read into a short int
      uint16_t unscaled_value = ((uint16_t *) block)[(channel == 0) ? 0 : 1];
      auto out_offset = &((uint16_t *) result->c_str())[i];
      *out_offset = ScaleAmplitude16(unscaled_value, bits_per_sample);
    }
    else if (sample_bits_ceiling == 24 || sample_bits_ceiling == 32) {
      // larger samples can be read into an int.
      uint32_t unscaled_value = 0;
      if (channel == 0) {
        unscaled_value = ((uint32_t *) block)[0];
      } else {
        unscaled_value = ((uint32_t *)((uint8_t *)block + block_align / 2))[0];
      }
      auto out_offset = &((uint32_t *) result->c_str())[i];
      *out_offset = ScaleAmplitude32(unscaled_value, bits_per_sample);
    }
  }

  return std::unique_ptr<std::string>(result);
}
