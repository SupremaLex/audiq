#include "audiq/audiq_similarity_model.h"
#include "gaia2/gaia.h"
#include "gaia2/view.h"
#include "gaia2/utils.h"
#include "gaia2/transformation.h"
#include "gaia2/distancefunction.h"
#include "audiq/audiq_util.h"
#include "audiq/audiq_types.h"
#include "audiq/audiq_config.h"

namespace audiq {
namespace similarity {
/* tricky init of gaia ( static objects creater before main function start) */
struct GaiaInit {
  GaiaInit() {
    gaia2::init();
  }
  ~GaiaInit() {
    gaia2::shutdown();
  }
};
static struct GaiaInit _gaia;

types::audiq_similar FindSimilar(DataSet *global_dataset, DataSet *user_dataset,
                                 const vector<float> &weights) {
  QStringList user_points = user_dataset->pointNames();
  DataSet* result_dataset = util::SumDataSets(global_dataset, user_dataset);
  DataSet* preprocessed = PreprocessDataSet(result_dataset);
  DataSet* processed = DefaultProcessDataSet(preprocessed);
  DistanceFunction* metric = DefaultMetric(processed,
                                           weights.at(0), weights.at(1), weights.at(2));
  return FindSimilar(processed, user_points, metric);
}

types::audiq_similar FindSimilar(DataSet *dataset, const QStringList &user_points,
                                 DistanceFunction *metric, int quantity) {
  gaia2::View v = gaia2::View(dataset);
  types::audiq_similar similar_samples;
  vector<string> temporary;
  int n = user_points.size();
  for ( auto p : user_points ) {
    temporary = vector<string>();
    auto result = v.nnSearch(p, metric);
    for ( auto pair : result.get(n + quantity) ) {
      // take only global dataset samples, even if samples from user dataser are more similar.
      if ( !user_points.contains(pair.first) ) {
        temporary.push_back(dataset->point(pair.first)
                            ->label(FILENAME_DESCRIPTOR)
                            .toSingleValue()
                            .toStdString());
        std::cout << pair.first.toStdString() << "   " << pair.second << std::endl;
      }
    }
    // take the most similar n samples
    vector<string> similar(temporary.begin(), temporary.begin() + quantity);
    similar_samples[dataset->point(p)
                    ->label(FILENAME_DESCRIPTOR)
                    .toSingleValue()
                    .toStdString()] = similar;
  }
  return similar_samples;
}

DataSet* PreprocessDataSet(DataSet *dataset) {
  ParameterMap enumerate_params;
  QStringList descriptors = {
    ".tonal.*.key*",
    ".tonal.*.scale*",
    ".tonal.*_key*",
    ".highlevel.*.value"
  };
  enumerate_params.insert("descriptorNames", descriptors);
  ParameterMap normalize_params;
  normalize_params.insert("except", QStringList() << "*mfcc*" << "metadata*");
  DataSet* enumerated = gaia2::transform(dataset, "Enumerate", enumerate_params);
  DataSet* cleaned    = gaia2::transform(enumerated, "Cleaner");
  delete enumerated;
  DataSet* normalized = gaia2::transform(cleaned, "Normalize");
  delete cleaned;
  return normalized;
}

DataSet* DefaultProcessDataSet(DataSet *dataset) {
  //
  ParameterMap select_metadata;
  select_metadata.insert("descriptorNames", "metadata.tags.file_name");
  DataSet* metadata = gaia2::transform(dataset, "Select", select_metadata);
  //
  ParameterMap select_mfcc;
  select_mfcc.insert("descriptorNames", QStringList() << "lowlevel.mfcc*");
  DataSet* mfcc = gaia2::transform(dataset, "Select", select_mfcc);
  //
  ParameterMap select_highlevel;
  select_highlevel.insert("descriptorNames", QStringList() << "highlevel*");
  DataSet* highlevel = gaia2::transform(dataset, "Select", select_highlevel);
  //
  DataSet* pca = Pca(dataset, QStringList() << "lowlevel.mfcc*" << "highlevel*", 25);
  DataSet* result = util::MergeDataSets({ metadata, mfcc, highlevel, pca });
  // to reduce memory usage delete datasets which we don't need
  delete pca;
  delete highlevel;
  delete mfcc;
  delete metadata;
  delete dataset;
  //
  return result;
}

DistanceFunction* DefaultMetric(DataSet *dataset, float weight_pca,
                                float weight_mfcc, float weight_highlevel) {
  ParameterMap pca;
  ParameterMap params_pca;
  params_pca.insert("descriptorNames", "pca");
  pca.insert("distance", "Euclidean");
  pca.insert("params", params_pca);
  pca.insert("weight", weight_pca);
  //
  ParameterMap mfcc;
  ParameterMap params_mfcc;
  params_mfcc.insert("descriptorName", "lowlevel.mfcc");
  mfcc.insert("distance", "KullbackLeibler");
  mfcc.insert("params", params_mfcc);
  mfcc.insert("weight", weight_mfcc);
  //
  ParameterMap highlevel;
  ParameterMap weights;
  for ( auto d : dataset->layout().descriptorNames(gaia2::RealType, QStringList() << "highlevel.*.all.*") ) {
    weights.insert(d, 1.0);
  }
  ParameterMap params_highlevel;
  params_highlevel.insert("weights", weights);
  highlevel.insert("distance", "WeightedPearson");
  highlevel.insert("params", params_highlevel);
  highlevel.insert("weight", weight_highlevel);
  // Linear combination of Euclidean and Kullback-Leibler  and Weighted Pearson metrics
  ParameterMap composite_params;
  composite_params.insert("euclidean_pca", pca);
  composite_params.insert("kullback_leibner_mfcc", mfcc);
  composite_params.insert("weighted_pearson_highlevel", highlevel);
  //
  return gaia2::MetricFactory::create("LinearCombination", dataset->layout(), composite_params);
}

DataSet* RemoveDescriptors(DataSet *dataset, const QStringList &descriptors) {
  ParameterMap remove_params;
  remove_params.insert("descriptorNames", descriptors);
  remove_params.insert("failOnUnmatched", false);
  return gaia2::transform(dataset, "Remove", remove_params);
}

DataSet* Pca(DataSet *dataset, const QStringList &except, int dimension) {
  ParameterMap pca_params;
  pca_params.insert("except", except);
  pca_params.insert("dimension", dimension);
  pca_params.insert("resultName", "pca");
  return gaia2::transform(dataset, "PCA", pca_params);
}

}  // namespace similarity
}  // namespace audiq
