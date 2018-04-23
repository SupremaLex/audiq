#ifndef PROJECT_AUDIQ_CONFIG_H
#define PROJECT_AUDIQ_CONFIG_H

#include <string>

#define GLOBAL_DATASET_NAME "audiq_dataset"
#define USER_DATASET_NAME   "user_dataset"
#define USER_DATASET_PART   "user_dataset_part"
#define DESCRIPTORS_DIR     "descriptors/"
#define DATASETS_DIR        "dataset_parts/"
#define MODELS_DIR          "svm_models/"

#define FILENAME_DESCRIPTOR "metadata.tags.file_name"
#define MD5_DESCRIPTOR      "metadata.audio_properties.md5_encoded"
#define TYPE_DESCRIPTOR     "highlevel.type.value"

#define SAMPLES_PER_DATASET 2000
#define QUANTITY            30

static const std::string MODEL_TYPE = "type.history";
static const std::string MODEL_PERCUSSION_TYPE = "percussion_type.history";
static const std::string MODEL_BASS = "bass.history";
static const std::string MODEL_SHOT_OR_LOOP = "shot_or_loop.history";
static const std::string MODEL_SYNTH_OR_ACOUSTIC = "synth_or_acoustic.history";
static const std::string MODEL_PHRASE = "phrase.history";

#endif  // PROJECT_AUDIQ_CONFIG_H

