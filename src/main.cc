#include "lame.h"

#include <iostream>
#include <fstream>
#include <memory>

#include "wav/header.hh"

void WaveToMp3() {
  lame_t flags = lame_init();
  if (flags) {
    printf("okok\n");
  }

  lame_close(flags);
}

int main() {
  std::ifstream input_file("test_data/sound03.wav");
  WavHeader wave_file(input_file);
  if (!wave_file.IsValidWav()) {
    printf("not a valid wave file\n");
    return 1;
  }

  WaveToMp3();
  printf("Hello world!\n");
  return 0;
}
