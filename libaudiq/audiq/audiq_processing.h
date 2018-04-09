#ifndef PROJECT_AUDIQ_PROCESSING_H
#define PROJECT_AUDIQ_PROCESSING_H

#include "audiq/audiq.h"
#include <string>
#include <vector>
#include "essentia/pool.h"
#include "audiq/audiq_types.h"
#include "audiq/audiq_config.h"

namespace audiq {
namespace processing {

using essentia::Pool;
/**
 * @brief SamplesToDataSet Gets dataset from samples of 'samples_directory'
 * @param samples_directory Directory with samples.
 * @param output_directory Directory with extracted files.
 * @param dataset_part_name Name of parts of dataset.
 * @param samples_per_dataset Number of samples in dataset part.
 * @param datasets_directory Directory with datasets parts.
 * @param dataset_name Name of result dataset.
 */
void SamplesToDataSet(const string &samples_directory,
                      const string &output_directory,
                      const string &profile,
                      const string &dataset_part_name,
                      const int samples_per_dataset,
                      const string &datasets_directory,
                      const string &dataset_name);
/**
 * @brief ProcessSamples Extracts descriptors of samples from 'samples_directory' store them into 'output_directory'
 * @param samples_directory Directory where samples stored
 * @param output_directory Directory where files with desciprots store
 */
void ProcessSamples(const string &samples_directory, const string &profile,
                    const string &output_directory, bool compute_highleve = true);

void ProcessSigsHighLevel(const string &directory);
/**
 * @brief Extract Extracts descriptors from single sample with name 'file_name' and stores them as 'output_file_name'
 * @param file_name Audio file (.wav, .aiff, .ogg, .mp3, mp4a).
 * @param profile Profile file with extractor config.
 * @param output_directory Name of result file with descriptors
 * @note Result file is YAML file
 */
void Extract(const string &file_name, const string &profile,
             const string &output_directory, bool compute_highlevel);

void ExtractLowLevel(Pool *pool, const string &file_name, const string &profile);

void ExtractHighLevel(const string &file_name);

void ExtractHighLevel(Pool *pool);

void ExtractHighLevel(Pool *pool, const vector<string> &models);

void InitializeMap();

}  // namespace processing
}  // namespace audiq
#endif  // PROJECT_AUDIQ_PROCESSING_H
