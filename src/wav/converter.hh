#ifndef WASHMYWAVES_WAV_CONVERTER_H__
#define WASHMYWAVES_WAV_CONVERTER_H__
#include <filesystem> // for std::filesystem::path

// @desc - converts a wav file to a mp3 file. the result will be saved 
//         under the same path and similar name with .mp3 extension.
// @param file_name - path to .wav file.
void ConvertWavToMP3(std::filesystem::path file_name);
#endif // WASHMYWAVES_WAV_CONVERTER_H__
