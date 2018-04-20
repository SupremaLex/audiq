#ifndef PROJECT_AUDIQ_AUDIQ_UTIL_H
#define PROJECT_AUDIQ_AUDIQ_UTIL_H

#include "audiq/audiq_util.h"
#include <string>
#include <vector>
#include <QVector>
#include <QStringList>
#include "gaia2/point.h"
#include "gaia2/dataset.h"
#include "audiq/audiq_types.h"
#include "audiq/audiq_config.h"

#define TYPE_DESCRIPTORS 1
#define TYPE_POINTS 2

namespace audiq {
namespace util {

using gaia2::Point;
using gaia2::DataSet;
struct Concatenate;
/**
 * @brief ConcatenateDataSets Concatenate datasets from 'datasets_directory' and save result dataset as 'dataset_name'.
 * @param datasets_directory Directory with datasets.
 * @param dataset_name Result dataset name.
 */
void ConcatenateDataSets(const string &datasets_directory = DATASETS_DIR,
                         const string &dataset_name = USER_DATASET_NAME);
/**
 * @brief MergeFiles Merges files with descriptors to datasets. Result of function - directory with datasets.
 *  Each dataset has name 'dataset_name' + number, and don't have more them 'samples_per_dataset' samples in it.
 * @param files_directory Directory with descriptors.
 * @param datasets_directory Directory with datasets parts.
 * @param dataset_name Name of the created dataset.
 * @param samples_per_dataset Number of samples in dataset chunk.
 */
void MergeFiles(const string &files_directory = DESCRIPTORS_DIR,
                const string &datasets_directory = DATASETS_DIR,
                const string &dataset_name = USER_DATASET_PART,
                const int samples_per_dataset = SAMPLES_PER_DATASET);
/**
 * SaveDataSetPart Creates dataset from 'samples' and save it as 'dataset_name'.
 */
void SaveDataSet(const QVector<Point*> &samples, const string &dataset_name);

void ReCreateDirs(const string &root_directory);

/**
 * LoadPoint Loads point from yaml file. If you don't need loaded point - free memory.
 */
Point* LoadPoint(const string &file_name, const string &point_name);

DataSet* PrepareDataSet(DataSet *dataset);

DataSet* Pca(DataSet *dataset, const QStringList &except, int dimension);
/**
 * MergeDataSets Merges datasets together, provided that their layout don't overlap, and
 * return the resulting dataset.
 */
DataSet* MergeDataSets(const vector<const DataSet*> &datasets);
/**
 * SumDataSets Sums datasets. The result dataset contains only intersection of there descriptors.
 */
DataSet* SumDataSets(const vector<DataSet*> &datasets);
/**
 * SumDataSets Sums two datasets. The result dataset contains only intersection of there descriptors.
 */
DataSet* SumDataSets(DataSet *first, DataSet *second);
/**
 * Intersection Find intersection of 'datasets' descriptors or point's (this depends on 'type')
 */
QStringList Intersection(const vector<DataSet*> &datasets, int type);
/**
 * Intersection Finds intersection of 'lists'.
 */
QStringList Intersection(const vector<QStringList> &lists);
/**
 * Intersection Finds intersection of two lists.
 */
QStringList Intersection(const QStringList &first, const QStringList &second);

/**
  Replace char 'ch' in str.
 */
string replaceStrChar(string str, const string& replace, char ch);

}  // namespace util
}  // namespace audiq
#endif  // PROJECT_AUDIQ_AUDIQ_UTIL_H
