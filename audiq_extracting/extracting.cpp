#include <string>
#include <iostream>
#include "audiq/audiq_processing.h"

int main(int argc, char* argv[]) {
  std::string dir_in = std::string(argv[1]);
  std::string dir_out = std::string(argv[2]);
  audiq::processing::ProcessSamples(dir_in, "", dir_out, false);
}
