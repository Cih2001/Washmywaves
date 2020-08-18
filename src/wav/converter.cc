#include <iostream>   // for writing to std io.
#include <fstream>    // for reading and writing files.
#include <memory>     // for smart pointers.
#include <string>

#include "lame.h"

#include "utils/global.hh"
#include "wav/header.hh"
#include "wav/converter.hh"

void ConvertWavToMP3(std::filesystem::path file_name) {
  printf("[DOING] %s\n", file_name.c_str());
  std::ifstream input_file(file_name);

  WavHeader wave_file(input_file);
  if (!wave_file.IsValidWav()) {
    printf("[ERROR] %s: not a valid wave file.\n", file_name.c_str());
    return;
  }

  auto fmt_header = wave_file.GetFormatChunkHeader();
  auto number_of_channels = fmt_header.number_of_channels;
  auto sample_rate = fmt_header.sample_rate;
  auto bits_per_sample = fmt_header.bits_per_sample;
  auto number_of_samples = wave_file.GetNumberOfSamples();
  auto audio_format = wave_file.GetAudioFormat();

  // mp3 format needs less space than wav.
  auto mp3_buff_size = GetFileSize(input_file);
  auto mp3_buff = std::unique_ptr<unsigned char>(
      new unsigned char[mp3_buff_size]);
  int bytes_written = 0;
  std::unique_ptr<std::string> left_channel, right_channel;

  PRINTF("audio format: %04x\n", (unsigned int)audio_format);
  PRINTF("number of channels: %d\n", (int)number_of_channels);
  PRINTF("sample rate: %d\n", (int)sample_rate);
  PRINTF("byte rate: %d\n", (int)fmt_header.byte_rate);
  PRINTF("block align: %d\n", (int)fmt_header.block_align);
  PRINTF("bits per sample: %d\n", (int)bits_per_sample);
  PRINTF("data size: 0x%x\n", (int)wave_file.GetDataSize());
  PRINTF("number of samples: 0x%x\n", (int)number_of_samples);
  PRINTF("data index: 0x%x\n", (int)wave_file.GetDataIndex());

  // check for supported versions.
  if (audio_format != WAVE_FORMAT_PCM &&
      audio_format != WAVE_FORMAT_IEEE_FLOAT) {
    printf("[ERROR] %s: unsupported audio format\n", file_name.c_str());
    return;
  }
  if (number_of_channels > 2 || bits_per_sample > 32) {
    printf("[ERROR] %s: unsupported audio format\n", file_name.c_str());
    return;
  }
  
  // initialize lame.
  lame_t flags = lame_init();
  if (!flags) {
    return;
  }
 
  lame_set_num_samples(flags, number_of_samples);
  lame_set_in_samplerate(flags, sample_rate);
  lame_set_num_channels(flags, number_of_channels);
  lame_set_quality(flags, 2); // 2 for the good quality. 0 is the best.

  if (lame_init_params(flags) < 0) {
    throw std::runtime_error("invalid lame parametrs.");
  }

  // TODO: check if there exists an .mp3 file with the same name.
  // and if yes, ask for the user permission to overwrite it.
  std::ofstream output_file(file_name.replace_extension(".mp3"));

  // read pcm data.
  if (number_of_channels == 1) {
    left_channel = wave_file.ReadPCMData(0);
    HEX_DUMP((const unsigned char *)left_channel->c_str(), 0x30);
  } else {
    left_channel = wave_file.ReadPCMData(0);
    right_channel = wave_file.ReadPCMData(1);
    HEX_DUMP((const unsigned char *)left_channel->c_str(), 0x30);
    HEX_DUMP((const unsigned char *)right_channel->c_str(), 0x30);
  }

  // encode pcm data.
  if (number_of_channels == 1) { // mono.
    if (audio_format == WAVE_FORMAT_PCM) {
      if (bits_per_sample <= 16) {
        bytes_written = lame_encode_buffer(
            flags,                                  // lame flags
            (const int16_t*)left_channel->c_str(),  // left channel buffer
            nullptr,                                // right channel buffer
            number_of_samples,                      // no of samples
            mp3_buff.get(),                         // output buffer
            mp3_buff_size);                         // output buffer size
      } else if (bits_per_sample > 16 && bits_per_sample <= 32) {
        bytes_written = lame_encode_buffer_int(
            flags,                                  // lame flags
            (const int32_t*)left_channel->c_str(),  // left channel buffer
            nullptr,                                // right channel buffer
            number_of_samples,                      // no of samples
            mp3_buff.get(),                         // output buffer
            mp3_buff_size);                         // output buffer size
      }
    } else if (audio_format == WAVE_FORMAT_IEEE_FLOAT) {
      bytes_written = lame_encode_buffer_ieee_float(
          flags,                                    // lame flags
          (const float*)left_channel->c_str(),      // left channel buffer
          nullptr,                                  // right channel buffer
          number_of_samples,                        // no of samples
          mp3_buff.get(),                           // output buffer
          mp3_buff_size);                           // output buffer size
    }
  } else if (number_of_channels == 2) { // stereo.
    if (audio_format == WAVE_FORMAT_PCM) {
      if (bits_per_sample <= 16) {
      bytes_written = lame_encode_buffer(
          flags,                                    // lame flags
          (const int16_t*)left_channel->c_str(),    // left channel buffer
          (const int16_t*)right_channel->c_str(),   // right channel buffer
          number_of_samples,                        // no of samples
          mp3_buff.get(),                           // output buffer
          mp3_buff_size);                           // output buffer size
      } else if (bits_per_sample > 16 && bits_per_sample <= 32) {
      bytes_written = lame_encode_buffer_int(
          flags,                                    // lame flags
          (const int32_t*)left_channel->c_str(),    // left channel buffer
          (const int32_t*)right_channel->c_str(),   // right channel buffer
          number_of_samples,                        // no of samples
          mp3_buff.get(),                           // output buffer
          mp3_buff_size);                           // output buffer size
      }
    } else if (audio_format == WAVE_FORMAT_IEEE_FLOAT) {
      bytes_written = lame_encode_buffer_ieee_float(
          flags,                                    // lame flags
          (const float*)left_channel->c_str(),      // left channel buffer
          (const float*)right_channel->c_str(),     // right channel buffer
          number_of_samples,                        // no of samples
          mp3_buff.get(),                           // output buffer
          mp3_buff_size);                           // output buffer size
    }

    // write encoded pcm data to mp3 file.
    output_file.write((const char *)mp3_buff.get(), bytes_written);
    bytes_written = lame_encode_flush(flags, mp3_buff.get(), mp3_buff_size);
    output_file.write((const char *)mp3_buff.get(), bytes_written);
    printf("[DONE ] %s\n", file_name.c_str());
  }

  lame_close(flags);
}
