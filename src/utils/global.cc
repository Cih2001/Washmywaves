#include <cstdio>

#include "utils/global.hh"

#ifdef DEBUG
void HEX_DUMP(const unsigned char* buffer, size_t size) {
  size_t i = 0;
  
  const unsigned int kOffsetSize = 10;
  char offsets[kOffsetSize] = {0};
  
  const unsigned int kHexStreamSize = 3 * 16 + 2;
  char hex_stream[kHexStreamSize] = {0};

  const unsigned int kTextStreamSize = 1 * 16 + 1;
  char text_stream[kTextStreamSize] = {0};
  while (i < size) {
    snprintf(offsets, kOffsetSize, "%08x:", (uint32_t)i);
    for (unsigned int j = 0; j < 16 && i + j < size; j++) {
      snprintf(&hex_stream[j*3], 4, "%02x ", buffer[i+j]);
    }
    for (unsigned int j = 0; j < 16 && i + j < size; j++) {
      auto chr = buffer[i+j] >= 0x20 && buffer[i+j] < 0x7f ? buffer[i+j] : '.';
      snprintf(&text_stream[j], 2, "%c", chr);
    }

    fprintf(stderr, "%s  %s  %s\n", offsets, hex_stream, text_stream);
    
    i += 16;
  }
  return;
}
#endif

size_t GetFileSize(std::ifstream& file) {
  if (!file.is_open()) {
    return 0;
  }

  auto previous_pos = file.tellg();
  file.seekg(0, std::ios::end);
  size_t file_size = file.tellg();
  file.seekg(previous_pos);
  return file_size;
}
