#include <filesystem>
#include <algorithm>
#include <pthread.h>
#include <vector>
#include "wav/converter.hh"

void* ThreadEntry(void* arg) {
  auto path = (const std::filesystem::path*)arg;
  ConvertWavToMP3(*path);
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("USAGE: washmywaves wav_files_directory\n");
    printf("  supported wav files:\n");
    printf("    - All types of PCM formats within 8-bits and 32-bits.\n");
    printf("    - IEEE float formats.\n");
    return 1;
  }

  std::vector<pthread_t> threads;
  std::vector<std::filesystem::path*> wav_files;

  // directory_iterator, introduced in C++17, is platform independant.
  // we do not need to worry about compilation in windows/linux.
  for (const auto &file : std::filesystem::directory_iterator(argv[1])) {
    if (file.path().has_extension()) { 
      auto extention = std::string(file.path().extension());
      // make extention lowercase.
      // TODO: we are assuming that file paths are ascii. this is a very
      // dangerous assumption.
      std::transform(extention.begin(), extention.end(), extention.begin(),
        [](unsigned char c) { return std::tolower(c); });
      if (extention == ".wav") {
        // parameters to threads are passed by reference. that's why we need
        // to store each path in a new variable and pass it to threads.
        wav_files.push_back(new std::filesystem::path(file.path()));
        // creating threads
        // for each wav file, we create a separate thread. we do not care how
        // many cores exist on the cpu and let the kernel handle multitasking.
        // if there are enough cores available, each thread will be run on a 
        // separate core.
        pthread_t new_thread;
        auto result = pthread_create(
            &new_thread,
            NULL,
            ThreadEntry,
            (void*) wav_files[wav_files.size()-1]);
        if (result) {
          // we really do not care about threads but we store them here anyway.
          threads.push_back(new_thread);
        }
      }
    }
  }

  pthread_exit(NULL);

  // freeing file paths, as they are allocated on heap.
  for (auto file : wav_files) {
    delete file;
  }
  return 0;
}
