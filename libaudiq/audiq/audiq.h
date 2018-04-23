#ifndef PROJECT_AUDIQ_AUDIQ_H
#define PROJECT_AUDIQ_AUDIQ_H

#include <map>
#include <string>
#include <vector>
#include "audiq/audiq_config.h"

namespace audiq {
typedef std::map<std::string, std::vector<std::string> > audiq_similar;

audiq_similar Recommend(const bool one_dataset,
                        const std::string &global_dataset_name,
                        const std::string &user_dataset_name,
                        const std::vector<float> &weights);
/**
 * PrintResult Prints result to stdout.
 */
void PrintResult(const audiq_similar &similars);

/**
 * GenerateResultFile Generates result YAML file, with key - "target" sample id and sequnce of ids corresponding to it.
 */
void GenerateResultFile(const audiq_similar &similars, const std::string &result_name);

namespace processing {

void SamplesToDataSet(const std::string &samples_directory,
                      const std::string &output_directory = DESCRIPTORS_DIR,
                      const std::string &profile = "",
                      const std::string &models_directory = MODELS_DIR,
                      const std::string &dataset_part_name = USER_DATASET_PART,
                      const int samples_per_dataset = SAMPLES_PER_DATASET,
                      const std::string &datasets_directory = DATASETS_DIR,
                      const std::string &dataset_name = USER_DATASET_NAME);

}  // namespace audiq
}  // namespace processing
#endif  // PROJECT_AUDIQ_AUDIQ_H

