#ifndef PROJECT_AUDIQ_TYPES_H
#define PROJECT_AUDIQ_TYPES_H

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <experimental/filesystem>
#include "audiq/audiq_config.h"

#define TYPE_PERCUSSION     "percussion"
#define TYPE_VOCAL          "vocal"
#define TYPE_MELODY         "melody"

namespace audiq {

using std::map;
using std::vector;
using std::string;
namespace filesystem = std::experimental::filesystem;

namespace types {

typedef std::map<std::string, std::vector<std::string> > audiq_similar;
static const std::vector<std::string> TYPES = { TYPE_PERCUSSION, TYPE_VOCAL, TYPE_MELODY };

}  // namespace types
}  // namespace audiq

#endif // PROJECT_AUDIQ_TYPES_H

