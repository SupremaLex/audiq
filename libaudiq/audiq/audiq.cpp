#include "audiq/audiq.h"
#include <ostream>
#include "yaml.h"
#include "audiq/audiq_util.h"
#include "audiq/audiq_types.h"
#include "audiq/audiq_config.h"
#include "audiq/audiq_similarity_model.h"

audiq::audiq_similar audiq::Recommend(const bool one_dataset, const string &global_dataset_name,
                                      const string &user_dataset_name, const vector<float> &weights) {
  using gaia2::DataSet;
  gaia2::init();
  vector<DataSet*> user_datasets;
  vector<DataSet*> global_datasets;
  audiq_similar similar;

  for ( auto t : types::TYPES ) {
    // if such file don't exists => there are no samples of this type in user' samples
    if ( !filesystem::exists(user_dataset_name + "_" + t + ".db") ) {
      continue;
    }
    DataSet* user  = new DataSet;
    DataSet* global = new DataSet;
    user->load(QString::fromStdString(user_dataset_name + "_" + t + ".db"));
    global->load(QString::fromStdString(global_dataset_name + "_" + t + ".db"));
    if ( !one_dataset ) {
      map<string, vector<string> > tmp = similarity::FindSimilar(global, user, weights);
      similar.insert(tmp.begin(), tmp.end());
      delete user;
      delete global;
      continue;
    }
    user_datasets.push_back(user);
    global_datasets.push_back(global);
  }
  if ( !one_dataset ) {
    return similar;
  }
  DataSet* united_user_dataset = util::SumDataSets(user_datasets);
  DataSet* united_global_dataset = util::SumDataSets(global_datasets);
  for ( auto d : user_datasets ) {
    delete d;
  }
  for ( auto d : global_datasets ) {
    delete d;
  }
  similar = similarity::FindSimilar(united_global_dataset, united_user_dataset, weights);
  delete united_global_dataset;
  delete united_user_dataset;
  return similar;
}

void audiq::PrintResult(const audiq_similar &similars) {
  for ( const auto &pair : similars ) {
    std::cout << pair.first << " :" << std::endl;
    for ( const auto &sample : pair.second ) {
      std::cout << "   " << sample << std::endl;
    }
    std::cout << std::endl;
  }
}

void audiq::GenerateResultFile(const audiq_similar &similars,
                               const std::string &result_name) {
  // Ugly C code for dumping map
  yaml_emitter_t emitter;
  yaml_document_t output_document;
  int sequence, item, mapping, key, seq;
  yaml_emitter_initialize(&emitter);
  FILE *output = fopen(result_name.c_str(), "wb");
  yaml_document_initialize(&output_document, NULL, NULL, NULL, 0, 0);
  yaml_emitter_set_output_file(&emitter, output);
  yaml_emitter_open(&emitter);
  const yaml_char_t* tmp;
  seq = yaml_document_add_sequence(&output_document, NULL,
                                        YAML_BLOCK_SEQUENCE_STYLE);
  for ( auto& pair : similars ) {
    mapping = yaml_document_add_mapping(&output_document, NULL, YAML_BLOCK_MAPPING_STYLE);
    yaml_document_append_sequence_item(&output_document, seq, mapping);
    sequence = yaml_document_add_sequence(&output_document, NULL,
                                          YAML_BLOCK_SEQUENCE_STYLE);
    tmp = reinterpret_cast<const yaml_char_t*>(pair.first.c_str());
    key      = yaml_document_add_scalar(&output_document, NULL,
                                        const_cast<yaml_char_t*>(tmp), -1,
                                        YAML_DOUBLE_QUOTED_SCALAR_STYLE);
    for ( auto& sample : pair.second ) {
      tmp = reinterpret_cast<const yaml_char_t*>(sample.c_str());
      item = yaml_document_add_scalar(&output_document, NULL,
                                      const_cast<yaml_char_t*>(tmp), -1,
                                      YAML_DOUBLE_QUOTED_SCALAR_STYLE);
      yaml_document_append_sequence_item(&output_document, sequence, item);
    }
    yaml_document_append_mapping_pair(&output_document, mapping, key, sequence);
  }
  yaml_emitter_dump(&emitter, &output_document);
  yaml_document_delete(&output_document);
  yaml_emitter_delete(&emitter);
}

