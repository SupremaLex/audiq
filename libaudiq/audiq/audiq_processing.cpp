#include "audiq/audiq_processing.h"
#include "audiq/audiq.h"
#include <map>
#include <set>
#include "essentia/algorithm.h"
#include "essentia/algorithmfactory.h"
#include "audiq/audiq_util.h"
#include "audiq/audiq_types.h"
#include "audiq/audiq_config.h"
#include "audiq_extractor/audiq_music_extractor.h"

namespace audiq {
namespace processing {

using essentia::standard::Algorithm;
using essentia::standard::AlgorithmFactory;

enum SampleType { percussion, vocal, melody };
map<string, SampleType> name_to_type;

void SamplesToDataSet(const string &samples_directory,
                      const string &output_directory,
                      const string &profile,
                      const string &models_directory,
                      const string &dataset_part_name,
                      const int samples_per_dataset,
                      const string &datasets_directory,
                      const string &dataset_name) {
  if ( !essentia::isInitialized() )
    essentia::init();
  // remove directories with data from previous session
  if ( filesystem::exists(filesystem::path(output_directory)) )
    filesystem::remove_all(output_directory);

  filesystem::create_directory(output_directory);
  util::ReCreateDirs(datasets_directory);
  ProcessSamples(samples_directory, profile, output_directory, models_directory);
  util::MergeFiles(output_directory, datasets_directory,
                   dataset_part_name, samples_per_dataset);
  for ( auto t : types::TYPES ) {
    util::ConcatenateDataSets(datasets_directory + "/" + t, dataset_name + "_" + t);
  }
}

void ProcessSamples(const string &directory, const string &profile,
                    const string &output_directory, const string &models_directory,
                    bool compute_highlevel) {
  if ( !essentia::isInitialized() )
    essentia::init();
  std::set<string> extensions = { ".wav", ".mp3", ".mp4a", ".ogg", ".aiff" };
  if (!filesystem::exists(filesystem::path(output_directory)))
    filesystem::create_directory(output_directory);
  for ( auto &p : filesystem::recursive_directory_iterator(directory) ) {
    std::cout << p << std::endl;
    auto ext = p.path().extension();
    if ( extensions.find(ext) != extensions.end() ) {
        Extract(p.path().string(), profile, output_directory, models_directory, compute_highlevel);
    }
  }
}

void Extract(const string &file_name, const string &profile,
             const string &output_directory, const string &models_directory,
             bool compute_highlevel) {
  Pool pool;
  try {
    ExtractLowLevel(&pool, file_name, profile);
    if ( compute_highlevel ) {
      ExtractHighLevel(&pool, models_directory);
    }
  }
  catch (essentia::EssentiaException e) {
    std::cout << e.what() << std::endl;
    return;
  }
  Algorithm* output = AlgorithmFactory::create("YamlOutput");
  output->input("pool").set(pool);
  output->configure("filename", output_directory + pool.value<string>(MD5_DESCRIPTOR) + ".sig");
  output->compute();
  delete output;
}

void ProcessSigsHighLevel(const string &directory, const string &models_directory) {
  if ( !essentia::isInitialized() )
    essentia::init();
  for ( auto &p : filesystem::recursive_directory_iterator(directory) ) {
    ExtractHighLevel(p.path().string(), models_directory);
  }
}

void ExtractLowLevel(Pool *pool, const string &file_name,
                     const string &profile) {
  Pool temporary_pool;
  Algorithm* extractor = new extractor::AudiqMusicExtractor;;
  InitializeMap();
  if ( filesystem::exists(profile) ) {
    extractor->configure("profile", profile);
  } else {
    extractor->configure("lowlevelSilentFrames", "noise",
                         "tonalSilentFrames", "noise");
  }
  extractor->input("filename").set(file_name);
  extractor->output("results").set(*pool);
  extractor->output("resultsFrames").set(temporary_pool);
  extractor->compute();
  delete extractor;
}

void ExtractHighLevel(const string &file_name, const string &models_directory) {
  Pool pool;
  Algorithm* input  = AlgorithmFactory::create("YamlInput", "filename", file_name);
  Algorithm* output = AlgorithmFactory::create("YamlOutput", "filename", file_name);
  input->output("pool").set(pool);
  input->compute();
  ExtractHighLevel(&pool, models_directory);
  output->input("pool").set(pool);
  output->compute();
}

void ExtractHighLevel(Pool *pool, const string &models_directory) {
  InitializeMap();
  vector<string> models_common = { models_directory + MODEL_BASS,
                                   models_directory + MODEL_SHOT_OR_LOOP,
                                   models_directory + MODEL_SYNTH_OR_ACOUSTIC };
  vector<string> model_type = { models_directory + MODEL_TYPE };
  vector<string> model_phrase = { models_directory + MODEL_PHRASE };
  vector<string> model_perc = { models_directory + MODEL_PERCUSSION_TYPE };
  pool->removeNamespace("highlevel");
  ExtractHighLevel(pool, model_type);
  switch ( name_to_type[pool->value<string>(TYPE_DESCRIPTOR)] ) {
  case vocal:
    ExtractHighLevel(pool, model_phrase);
    break;
  case percussion:
    ExtractHighLevel(pool, model_perc);
  default:
    ExtractHighLevel(pool, models_common);
    break;
  }
}

void ExtractHighLevel(Pool *pool, const vector<string> &models) {
  Algorithm* extractor_svm = AlgorithmFactory::create("MusicExtractorSVM", "svms", models);
  try {
    extractor_svm->input("pool").set(*pool);
    extractor_svm->output("pool").set(*pool);
    extractor_svm->compute();
  }
  catch ( essentia::EssentiaException ) {
  }
  delete extractor_svm;
}

void InitializeMap() {
  name_to_type[TYPE_VOCAL] = vocal;
  name_to_type[TYPE_MELODY] = melody;
  name_to_type[TYPE_PERCUSSION] = percussion;
}

}  // namespace processing
}  // namespace audiq
