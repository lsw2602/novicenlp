#ifndef HMM_LEARNHMM_MODEL_OBSERVATIONMODEL_H_
#define HMM_LEARNHMM_MODEL_OBSERVATIONMODEL_H_

#include <stdio.h>
#include <string>
#include <map>

#include "common/typedef.h"

namespace hmm {

class TrainingSetParser;

class ObservationModel {
  // LabelType (std::string), OutputType (std::string)
  typedef std::map<OutputType, int> LabelModelType;
  typedef std::map<LabelType, std::pair<int, LabelModelType>> ModelType;
public:
  ObservationModel();
  ~ObservationModel();
  
  bool ReadModel(std::string model_path, HMMErrorType &status);
  bool ReadModel(FILE* fin, HMMErrorType &status);
  bool WriteModel(std::string model_path, HMMErrorType &status);
  bool WriteModel(FILE* fout, HMMErrorType &status);
  bool Learn(TrainingSetParser &parser, HMMErrorType &status);

  ScoreType GetScore(LabelType state, OutputType output, HMMErrorType &status);
  ScoreType GetOOVScore();

  void Clear();

public:
  static const std::string observation_open_tag_;
  static const std::string observation_close_tag_;
  static const std::string state_open_tag_;
  static const std::string state_close_tag_;
  static const std::string count_delimiter_;

private:
  ModelType model_;
  ScoreType oov_score_;
};

} // namespace hmm

#endif