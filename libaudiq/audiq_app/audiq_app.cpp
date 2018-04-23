#include "audiq_app.h"
#include "essentia/essentia.h"
#include "essentia/algorithmfactory.h"
#include "gaia2/gaia.h"
#include "audiq/audiq.h"
#include "audiq/audiq_processing.h"

using audiq::Audiq;
using namespace std;
using namespace essentia;

Audiq::Audiq() {
  declareParameters();
  essentia::init();
  gaia2::init();
}

void Audiq::declareParameters() {
  declareParameter("samples_directory", "Directory where samples stored", "", Parameter::STRING);
  declareParameter("global_dataset_name", "The name of global dataset", "", "audiq_dataset");
  declareParameter("user_dataset_name", "The name of user dataset", "", "user_dataset");
  declareParameter("descriptors_directory", "Directory where descriptors files stored", "", "descriptors/");
  declareParameter("datasets_parts_directory", "Directory where datasets parts stored", "", "datasets_parts/");
  declareParameter("svm_models_directory", "Directory where svm models stored", "", "svm_models/");
  declareParameter("extractor_profile", "Essentia  MusicExtractor profile", "", "");
  declareParameter("audiq_profile", "Audiq profile", "", Parameter::STRING);
  declareParameter("dataset_mode", "Audiq mode", "{one, many}", "many");
  declareParameter("only_recommendation", "Don't process samples and datasets creating", "{true, false}", false);
  declareParameter("samples_in_dataset", "Number of samples in dataset part", "(10,inf)", 2000);
  declareParameter("recommended_samples_number", "Number of the most similar samples to recommend", "(10, inf)", 30);
  declareParameter("weight_lowlevel", "Weight corresponding to lowlevel component in metric used bu audiq", "(0, 10)", 1.0);
  declareParameter("weight_timbre", "Weight corresponding to timbre component in metric used bu audiq", "(0, 10)", 1.0);
  declareParameter("weight_highlevel", "Weight corresponding to highlevel component in metric used bu audiq", "(0, 10)", 1.0);
}

void Audiq::configure() {
  _global_dataset_name = parameter("global_dataset_name").toString();
  _user_dataset_name = parameter("user_dataset_name").toString();
  _descriptors_directory = parameter("descriptors_directory").toString();
  _datasets_parts_directory = parameter("datasets_parts_directory").toString();
  _svm_models_directory = parameter("svm_models_directory").toString();
  _dataset_mode = parameter("dataset_mode").toString();
  _extractor_profile = parameter("extractor_profile").toString();
  _only_recommendation = parameter("only_recommendation").toBool();
  _samples_in_dataset = parameter("samples_in_dataset").toInt();
  _recommended_samples_number = parameter("recommended_samples_number").toInt();
  _weight_lowlevel = parameter("weight_lowlevel").toFloat();
  _weight_timbre = parameter("weight_timbre").toFloat();
  _weight_highlevel = parameter("weight_highlevel").toFloat();
  if ( parameter("samples_directory").isConfigured() ) {
    _samples_directory = parameter("samples_directory").toString();
  }
  _options.clear();
  SetDefaultOptions();
  if ( parameter("audiq_profile").isConfigured() ) {
    SetOptions(parameter("audiq_profile").toString());
  }
}

void Audiq::SetDefaultOptions() {
  _options.set("global_dataset_name", _global_dataset_name);
  _options.set("user_dataset_name", _user_dataset_name);
  _options.set("descriptors_directory", _descriptors_directory);
  _options.set("datasets_parts_directory", _datasets_parts_directory);
  _options.set("svm_models_directory", _svm_models_directory);
  _options.set("dataset_mode", _dataset_mode);
  _options.set("extractor_profile", _extractor_profile);
  _options.set("only_recommendation", _only_recommendation);
  _options.set("samples_in_dataset", _samples_in_dataset);
  _options.set("recommended_samples_number", _recommended_samples_number);
  _options.set("weight_lowlevel", _weight_lowlevel);
  _options.set("weight_timbre", _weight_timbre);
  _options.set("weight_highlevel", _weight_highlevel);
}

void Audiq::SetOptions(string file_name) {
  if (file_name.empty()) return;
  Pool opts;
  standard::Algorithm * yaml = standard::AlgorithmFactory::create("YamlInput", "filename", file_name);
  yaml->output("pool").set(opts);
  yaml->compute();
  delete yaml;
  _options.merge(opts, "replace");
}

void Audiq::Start() {
  bool dataset_mode = false;
  if ( _options.value<string>("dataset_mode").compare("one") == 0 )
    dataset_mode = true;
  if ( !_only_recommendation ) {
  processing::SamplesToDataSet(_samples_directory,
                               _options.value<string>("descriptors_directory"),
                               _options.value<string>("extractor_profile"),
                               _options.value<string>("svm_models_directory"),
                               _options.value<string>("user_dataset_name") + "part",
                               _options.value<Real>("samples_in_dataset"),
                               _options.value<string>("datasets_parts_directory"),
                               _options.value<string>("user_dataset_name"));
  }
  _similar_samples =  Recommend(dataset_mode, _options.value<string>("global_dataset_name"),
                                _options.value<string>("user_dataset_name"), {
                                  _options.value<float>("weight_lowlevel"),
                                  _options.value<float>("weight_timbre"),
                                  _options.value<float>("weight_highlevel") });
}

audiq::types::audiq_similar Audiq::GetResult() {
  return _similar_samples;
}

string Audiq::GetMode() {
  return _dataset_mode;
}
