#ifndef PROJECT_AUDIQ_SIMILARITY_MODEL_H
#define PROJECT_AUDIQ_SIMILARITY_MODEL_H
#include <string>
#include <vector>
#include <QStringList>
#include "gaia2/dataset.h"
#include "gaia2/parameter.h"
#include "audiq/audiq_types.h"
#include "audiq/audiq_config.h"

namespace audiq {
namespace similarity {

using gaia2::DataSet;
using gaia2::ParameterMap;
using gaia2::DistanceFunction;

types::audiq_similar FindSimilar(DataSet *global_dataset, DataSet *user_dataset, const vector<float> &weights);
/**
 * @brief FindSimilar Find 'quantity' the most similar samples in the 'audiq_dataset' for samples from 'user_dataset'
 * @param global_dataset DataSet where Audiq samples stored.
 * @param user_points Samples id's for which is need to find similar. This id's must be in 'global_dataset'.
 * @param metric Metric used for similarity measure.
 * @param quantity Number of the most similar samples to return.
 * @return map with user samples names and corresponding to them the most similar samples from 'audiq_dataset'.
 * @note Function find the most similar samples in 'global_dataset' and return samples from it
 *  even if there are some samples in 'user_dataset' which more similar.
 * The original 'global_dataset' is deleted during function work (because of high memory usage).
 */
types::audiq_similar FindSimilar(DataSet *dataset, const QStringList &user_points,
                                         DistanceFunction *metric, int quantity = QUANTITY);
/**
 * @brief PreprocessDataSet Apply preprocessing to the dataset.
 * @param dataset Original dataset.
 * @return Preprocessed dataset.
 *  Preprocessing consist of applying of Enumerate, Cleaner and Normalize transformations.
 */
DataSet* PreprocessDataSet(DataSet *dataset);

DataSet* DefaultProcessDataSet(DataSet *dataset);

DistanceFunction* DefaultMetric(DataSet *dataset, float weight_pca, float weight_mfcc, float weight_highlevel);
/**
 * @brief RemoveDescriptors Removes 'descriptors' from dataset.
 * @param dataset Original dataset.
 * @param descriptors Descriptors to be deleted from dataset.
 * @return New dataset without 'descriptors'.
 * @note Original dataset saved untoughed.
 */
DataSet* RemoveDescriptors(DataSet *dataset, const QStringList &descriptors);

/**
  * @brief PCA Apply PCA transformation to dataset.
  * @param dataset Original dataset.
  * @param except Don't use this descriptors.
  * @param dimension Target dimension.
  * @return New dataset.
  * @note Original dataset saved untoughed.
  */
DataSet* Pca(DataSet *dataset,  const QStringList &except, int dimension);

}  // namespace similarity
}  // namespace audiq
#endif  // PROJECT_AUDIQ_SIMILARITY_MODEL_H
