#ifndef WASHMYWAVES_UTILS_GLOBAL_H__
#define WASHMYWAVES_UTILS_GLOBAL_H__

#ifdef DEBUG
void HEX_DUMP(const unsigned char* buffer, size_t size);
#else
#define HEX_DUMP(b, s)
#endif

#endif // WASHMYWAVES_UTILS_GLOBAL_H__
