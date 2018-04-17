#include "audiq/audiq_util.h"
#include <map>
#include "gaia2/utils.h"
#include "audiq/audiq_config.h"
#include "audiq/audiq_types.h"

namespace audiq {
namespace util {

using gaia2::ParameterMap;
namespace filesystem = std::experimental::filesystem;

void ConcatenateDataSets(const string &datasets_directory, const string &dataset_name) {
  DataSet ds;
  DataSet* result = &ds;
  bool first = true;
  for ( auto& p : filesystem::recursive_directory_iterator(datasets_directory) ) {
    if ( first ) {
      result->load(QString::fromStdString(p.path().string()));
      result->forgetHistory();
      first = false;
      continue;
    }
    DataSet loaded;
    loaded.load(QString::fromStdString(p.path().string()));
    loaded.forgetHistory();
    result->appendDataSet(&loaded);
  }
  if ( !result->pointNames().empty() )
    result->save(QString::fromStdString(dataset_name + ".db"));
}

void MergeFiles(const string &files_directory, const string &datasets_directory,
                const string &dataset_name, const int n) {
  ReCreateDirs(datasets_directory);
  string name;
  string type;
  map<string, QVector<Point*> > samples;
  map<string, int> counters;
  for ( auto t : types::TYPES ) {
    samples[t] = QVector<Point*>();
    counters[t] = 0;
  }
  for ( auto& p : filesystem::recursive_directory_iterator(files_directory) ) {
    if ( p.path().extension() != ".sig" ) {
      continue;
    }
    for ( auto& pair : counters ) {
      if ( pair.second % n == 0 && !samples[pair.first].empty() ) {
        name = datasets_directory + pair.first + "/" + pair.first + "_"
             + dataset_name + std::to_string(pair.second / n) + ".db";
        SaveDataSet(samples[pair.first], name);
        for ( auto s : samples[pair.first] )
          delete s;
        samples[pair.first].clear();
        }
    }
    Point* sample = LoadPoint(p.path().string(), p.path().filename().string());
    type = sample->label(TYPE_DESCRIPTOR).toSingleValue().toStdString();
    samples[type] << sample;
    ++counters[type];
  }
  for ( auto pair : samples ) {
    if ( !pair.second.empty() ) {
      name = datasets_directory + "/" + pair.first + "/" + pair.first + "_"
           + dataset_name + std::to_string(pair.second.size() / n + 1) + ".db";
      SaveDataSet(pair.second, name);
      for ( auto s : pair.second )
        delete s;
      pair.second.clear();
      }
  }
}

void SaveDataSet(const QVector<Point*> &samples, const string &dataset_name) {
  DataSet dataset;
  dataset.addPoints(samples);
  // Some transformations to reduce memory usage in future
  DataSet* prepared = PrepareDataSet(&dataset);
  prepared->setName(QString::fromStdString(dataset_name));
  prepared->save(QString::fromStdString(dataset_name));
  delete prepared;
}


void ReCreateDirs(const string &root_directory) {
  for ( auto t : types::TYPES ) {
    if ( filesystem::exists(filesystem::path(root_directory + "/" + t)) )
      filesystem::remove_all(root_directory + "/" + t);
    }
  if ( !filesystem::exists(filesystem::path(root_directory)) )
    filesystem::create_directory(root_directory);
  for ( auto t : types::TYPES ) {
      filesystem::create_directory(root_directory + "/" + t);
    }
}

Point* LoadPoint(const string &file_name, const string &point_name) {
  Point* point = new Point;
  point->load(QString::fromStdString(file_name));
  point->setName(QString::fromStdString(point_name));
  /* different codecs have different number and types of additional information, such tags,
  so we will only take file_name as tags. Because of QRegex Wildcard's implementation, we can't use [!f]...
  QStringList tags = { ".metadata.tags.[qwertyuiopasdghjklzxcvbnm]*" };
  if ( !point->layout().descriptorNames(gaia2::StringType, tags, false).empty() ) {
    point->layout().filter(QStringList() << "*", tags);
    point->setLayout(point->layout());
  }*/
  return point;
}

DataSet* PrepareDataSet(DataSet *dataset) {
  ParameterMap enumerate_params;
  QStringList enumerate = {
    ".tonal.*.key*",
    ".tonal.*.scale*",
    ".tonal.*_key*",
    ".highlevel.*.value"
  };
  enumerate_params.insert("descriptorNames", enumerate);
  DataSet* removed_vl = gaia2::transform(dataset, "RemoveVL");
  DataSet* fixed_length = gaia2::transform(removed_vl, "FixLength");
  delete removed_vl;
  DataSet* transformed = gaia2::transform(fixed_length, "Enumerate", enumerate_params);
  delete fixed_length;
  //
  ParameterMap select_metadata;
  select_metadata.insert("descriptorNames", "metadata.tags.file_name");
  DataSet* metadata = gaia2::transform(transformed, "Select", select_metadata);
  //
  ParameterMap select_mfcc;
  select_mfcc.insert("descriptorNames", QStringList() << "lowlevel.mfcc*");
  DataSet* mfcc = gaia2::transform(transformed, "Select", select_mfcc);
  //
  ParameterMap select_highlevel;
  select_highlevel.insert("descriptorNames", QStringList() << "highlevel*");
  DataSet* highlevel = gaia2::transform(transformed, "Select", select_highlevel);
  //
  ParameterMap normalize_params;
  normalize_params.insert("except", QStringList() << "lowlevel.mfcc*" << "highlevel*");
  DataSet* cleaned    = gaia2::transform(transformed, "Cleaner");
  DataSet* normalized = gaia2::transform(cleaned, "Normalize", normalize_params);
  DataSet* pca = Pca(normalized, QStringList() << "lowlevel.mfcc*" << "highlevel*", 25);
  DataSet* result = MergeDataSets({ metadata, mfcc, highlevel, pca });
  // to reduce memory usage delete datasets which we don't need
  delete pca;
  delete normalized;
  delete cleaned;
  delete highlevel;
  delete mfcc;
  delete metadata;
  delete transformed;
  //
  return result;
}

DataSet* Pca(DataSet *dataset, const QStringList &except, int dimension) {
  ParameterMap pca_params;
  pca_params.insert("except", except);
  pca_params.insert("dimension", dimension);
  pca_params.insert("resultName", "pca");
  return gaia2::transform(dataset, "PCA", pca_params);
}

DataSet* MergeDataSets(const vector<DataSet*> &datasets) {
  DataSet* result = datasets[0];
  for ( int i = 1; i < static_cast<int>(datasets.size()); ++i ) {
    result = gaia2::mergeDataSets(result, datasets[i]);
  }
  return result;
}

DataSet* SumDataSets(const vector<DataSet*> &datasets) {
  DataSet* previous = datasets[0];
  DataSet* result;
  for ( int i = 1; i < static_cast<int>(datasets.size()); ++i ) {
    result = SumDataSets(previous, datasets[i]);
    if ( i > 1)
      delete previous;
    previous = result;
  }
  return result;
}

DataSet* SumDataSets(DataSet *first, DataSet *second) {
  QStringList common_descriptors = Intersection(first->layout().descriptorNames(),
                                                second->layout().descriptorNames());
  QStringList points_intersection = Intersection(first->pointNames(),
                                                 second->pointNames());
  ParameterMap select_params;
  select_params.insert("descriptorNames", common_descriptors);
  DataSet* selected_first = first;
  DataSet* selected_second = second;
  // first of all - delete intersection of points from one of datasets
  selected_second->removePoints(points_intersection);
  bool free = false;
  if ( first->layout().descriptorNames() != common_descriptors ) {
    selected_first  = gaia2::transform(first, "Select", select_params);
  }
  if ( first->layout().descriptorNames() != common_descriptors ) {
    selected_second = gaia2::transform(second, "Select", select_params);
    free = true;
  }
  selected_first->forgetHistory();
  selected_second->forgetHistory();
  selected_first->appendDataSet(selected_second);
  if ( free ) {
    delete selected_second;
  }
  return selected_first;
}

QStringList Intersection(const vector<DataSet*> &datasets, int type) {
  vector<QStringList> data;
  for ( auto d : datasets ) {
    if ( type == TYPE_DESCRIPTORS ) {
      data.push_back(d->layout().descriptorNames());
    } else if ( type == TYPE_POINTS ) {
            data.push_back(d->pointNames());
    }
  }
  return Intersection(data);
}

QStringList Intersection(const vector<QStringList> &lists) {
  QStringList result = lists[0];
  for ( int i = 1; i < static_cast<int>(lists.size()); ++i ) {
    result = Intersection(result, lists[i]);
  }
  return result;
}

QStringList Intersection(const QStringList &first, const QStringList &second) {
  return first.toSet().intersect(second.toSet()).toList();
}

string replaceStrChar(string str, const string& replace, char ch) {
  size_t found = str.find_first_of(replace);
  while ( found != string::npos ) {
    str[found] = ch;
    found = str.find_first_of(replace, found+1);
  }
  return str;
}

}  // namespace util
}  // namespace audiq
