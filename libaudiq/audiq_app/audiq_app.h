#ifndef AUDIQ_APP_H
#define AUDIQ_APP_H

#include <map>
#include <string>
#include <vector>
#include "essentia/essentia.h"
#include "essentia/pool.h"
#include "essentia/configurable.h"
#include "audiq/audiq_types.h"

namespace audiq {

class Audiq: public essentia::Configurable {
 public:
   Audiq();
   using essentia::Configurable::configure;
   void configure();
   void declareParameters();
   void SetDefaultOptions();
   void SetOptions(std::string file_name);
   void Start();
   types::audiq_similar GetResult();
   std::string GetMode();

 private:
   std::string _samples_directory;
   std::string _global_dataset_name;
   std::string _user_dataset_name;
   std::string _descriptors_directory;
   std::string _datasets_parts_directory;
   //std::string _result_file_name;
   std::string _svm_models_directory;
   std::string _extractor_profile;
   std::string _dataset_mode;

   bool _only_recommendation;

   int _samples_in_dataset;
   int _recommended_samples_number;
   float _weight_lowlevel;
   float _weight_timbre;
   float _weight_highlevel;

   types::audiq_similar _similar_samples;
   essentia::Pool _options;
};

} // audiq
#endif // AUDIQ_APP_H
