#ifndef WASHMYWAVES_UTILS_GLOBAL_H__
#define WASHMYWAVES_UTILS_GLOBAL_H__
#include <cinttypes>
#include <fstream>

#ifdef DEBUG
// @desc - dumps the content of a buffer on std io in hexadecimal format.
//         used for debugging purposes.
// @param buffer - buffer to be dumped.
// @param size - size of data to be dumped. there exists no boundary check.
void HEX_DUMP(const unsigned char* buffer, size_t size);
#else
#define HEX_DUMP(b, s) // do nothing in release builds
#endif

#ifdef DEBUG
  #include <stdio.h> // for perror, stderr, printf and etc.
  #define PRINTF(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
          __FILE__, __LINE__, __func__, ##args) // Pretty print debug messages
#else
  #define PRINTF(...) // do nothing in release builds
#endif


size_t GetFileSize(std::ifstream& file);

#endif // WASHMYWAVES_UTILS_GLOBAL_H__
