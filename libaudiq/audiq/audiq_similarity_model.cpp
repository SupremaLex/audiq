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


types::audiq_similar FindSimilar(DataSet *global_dataset, DataSet *user_dataset,
                                 const vector<float> &weights) {
  gaia2::init();
  QStringList user_points = user_dataset->pointNames();
  DataSet* result_dataset = util::SumDataSets(global_dataset, user_dataset);
  DistanceFunction* metric = CompressedDefaultMetric(result_dataset,
                                                     weights.at(0), weights.at(1), weights.at(2));
  return FindSimilar(result_dataset, user_points, metric);
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
      if ( !user_points.contains(pair.first)) {
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

DistanceFunction* CompressedDefaultMetric(DataSet *dataset, float weight_pca,
                                          float weight_mfcc, float weight_highlevel) {

  ParameterMap pca;
  ParameterMap compressed_pca;
  ParameterMap params_pca;
  params_pca.insert("descriptorNames", "pca");
  compressed_pca.insert("distance", "Euclidean");
  compressed_pca.insert("params", params_pca);
  compressed_pca.insert("alpha", 0.01);
  pca.insert("distance", "ExponentialCompress");
  pca.insert("params", compressed_pca);
  pca.insert("weight", weight_pca);
  //
  ParameterMap mfcc;
  ParameterMap compressed_mfcc;
  ParameterMap params_mfcc;
  params_mfcc.insert("descriptorName", "lowlevel.mfcc");
  compressed_mfcc.insert("distance", "KullbackLeibler");
  compressed_mfcc.insert("params", params_mfcc);
  compressed_mfcc.insert("alpha", 0.01);
  mfcc.insert("distance", "ExponentialCompress");
  mfcc.insert("params", compressed_mfcc);
  mfcc.insert("weight", weight_mfcc);
  //
  ParameterMap highlevel;
  ParameterMap compressed_highlevel;
  ParameterMap weights;
  for ( auto d : dataset->layout().descriptorNames(gaia2::RealType, QStringList() << "highlevel.*.all.*") ) {
   weights.insert(d, 1.0);
  }
  ParameterMap params_highlevel;
  params_highlevel.insert("weights", weights);
  compressed_highlevel.insert("distance", "WeightedPearson");
  compressed_highlevel.insert("params", params_highlevel);
  compressed_highlevel.insert("alpha", 0.01);
  highlevel.insert("distance", "ExponentialCompress");
  highlevel.insert("params", compressed_highlevel);
  highlevel.insert("weight", weight_highlevel);
  // Linear combination of Euclidean and Kullback-Leibler and Weighted Pearson metrics

  ParameterMap composite_params;
  composite_params.insert("compressed_euclidean_pca", pca);
  composite_params.insert("compressed_kullback_leibner_mfcc", mfcc);
  composite_params.insert("compressed_weighted_pearson_highlevel", highlevel);
  //

  return gaia2::MetricFactory::create("LinearCombination", dataset->layout(), composite_params);
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
  // Linear combination of Euclidean and Kullback-Leibler and Weighted Pearson metrics
  ParameterMap composite_params;
  composite_params.insert("euclidean_pca", pca);
  composite_params.insert("kullback_leibner_mfcc", mfcc);
  composite_params.insert("weighted_pearson_highlevel", highlevel);
  //

  return gaia2::MetricFactory::create("LinearCombination", dataset->layout(), composite_params);
}


}  // namespace similarity
}  // namespace audiq
