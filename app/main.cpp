#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include "getopt.h"
#include "audiq/audiq.h"

using std::string;
using std::cout;
using std::endl;

void options() {
  cout << "Usage: ./audiq [options] folder_with_samples [output_file] [weight_1 weight_2 weight_3]\n"
       << "Notes:\nIf 'output_file' arg was not parsed"
       <<  " then result file name would be 'result_w1_w2_w3_[one|many]_ds.yaml'.\n"
       << "weight_1, weight_2, weight_3 (weight coefficients in metric used by audiq)"
       << " corresponds to low-level signal descriptors (such energy...), timbre"
       << " and highlevel descriptors (such as is this sample percussion or vocal)"
       << " default values fo weights 1, 2, 3.\n"
       << "Options:\n"
       << "\t-h, --help Show this message.\n"
       << "\t-o, --one  Use one_dataset mode (find similar for each sample not only in it's type dataset).\n"
       << "\t-p, --print  Print result to stdout.\n"
       << "\t-n, --no-processing Don't extract descriptors or datasets creating"
       << " (use this option if you have datasets and want test audiq with different combinations of modes and weights,"
       << " expected that you have datasets).\n"
       << endl;
}

int main(int argc, char* argv[]) {
  string sounds_directory;
  string output_file;
  bool similarity_mode = false;
  bool print = false;
  bool processing = true;
  std::vector<float> weights = {1, 2, 3};
  int c;
  static struct option long_options[] = {
  {"help", no_argument, 0, 'h'},
  {"one", no_argument, 0, 'o'},
  {"print", no_argument , 0, 'p'},
  {"no-processing", no_argument, 0, 'n'},
  {0, 0, 0, 0}
  };
  while ( true ) {
    int option_index = 0;
    c = getopt_long(argc, argv, "hopn", long_options, &option_index);
    if (c == -1)
       break;
    switch (c) {
    case 'p':
        print = true;
        break;
    case 'h':
      options();
      break;
    case 'o':
      similarity_mode = true;
      break;
    case 'n':
      processing = false;
    }
  }
  switch ( argc - optind ) {
    case 1:
      sounds_directory = argv[argc - 1];
      break;
    case 2:
      sounds_directory = argv[argc - 2];
      output_file = argv[argc - 1];
      break;
    case 4:
      sounds_directory = argv[argc -4];
      weights.clear();
      weights.push_back(std::stof(string(argv[argc - 3])));
      weights.push_back(std::stof(string(argv[argc - 2])));
      weights.push_back(std::stof(string(argv[argc - 1])));
      break;
    case 5:
      sounds_directory = argv[argc - 5];
      output_file = argv[argc - 4];
      weights.clear();
      weights.push_back(std::stof(string(argv[argc - 3])));
      weights.push_back(std::stof(string(argv[argc - 2])));
      weights.push_back(std::stof(string(argv[argc - 1])));
      break;
    default:
      cout << "Wrong number of arguments\n";
      options();
      exit(1);
  }
  if ( processing ) {
    audiq::processing::SamplesToDataSet(sounds_directory);
  }
  audiq::audiq_similar result = audiq::Recommend(similarity_mode, weights);
  if ( print ) {
    audiq::PrintResult(result);
  }
  if ( output_file.empty() ) {
    output_file = "result";
    char buf[100];
    std::snprintf(buf, 100, "%.01f_%.01f_%.01f", weights[0], weights[1], weights[2]);
    output_file += "_" + string(buf);
    if ( similarity_mode ) {
      output_file += "_one_ds_mode";
    } else {
      output_file += "_many_ds_mode";
    }
    output_file += ".yaml";
  }
  audiq::GenerateResultFile(result, output_file);
  return 0;
}
