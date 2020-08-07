#include "wav/header.hh"
#include "utils/global.hh"

#define RIFF_CHUNK_ID 0x46464952
#define RIFF_FORMAT_WAVE 0x45564157
#define FMT_CHUNK_ID 0x20746d66

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
 
  // TODO: we assume that fmt chunk comes directly after the riff_header.
  // this is not assumption is not mandatory and is not enforced by RFC,
  // and should be fixed.
  auto fmt_chunk = CastBytes<FmtChunk>(input_);
  if (!input_) {
    // stream not long enough or an internal error in stream.
    return false;
  }
  HEX_DUMP((const unsigned char *)&fmt_chunk, sizeof(fmt_chunk));
  if (fmt_chunk.chunk_header.id != FMT_CHUNK_ID) {
    return false;
  }

  // TODO: we need more checks, like presence of data, chunks size and etc.
  return true;
}

