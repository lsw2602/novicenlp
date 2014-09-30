#include "learnhmm/model/observation_model.h"

#include "learnhmm\parser\trainingset_parser.h"
#include "common/utils.h"

namespace hmm {
  
const std::string ObservationModel::observation_open_tag_ = "<observation_model>";
const std::string ObservationModel::observation_close_tag_ = "</observation_model>";
const std::string ObservationModel::state_open_tag_ = "<state>";
const std::string ObservationModel::state_close_tag_ = "</state>";
const std::string ObservationModel::count_delimiter_ = "\t";

ObservationModel::ObservationModel() {
  oov_score_ = log(0.0);
}

ObservationModel::~ObservationModel() {
}
  
bool ObservationModel::ReadModel(std::string model_path, HMMErrorType &status) {
  FILE* fin = fopen(model_path.c_str(), "r");
  if (!fin) {
    status = kObservationErrorFileOpenFail;
    return false;
  }
  return ReadModel(fin, status);
}
bool ObservationModel::ReadModel(FILE* fin, HMMErrorType &status) {
  status = kObservationSuccess;
  HMM_FLAG obs_flag = FLAG_OFF, state_flag = FLAG_OFF;
  
  if (fin == NULL) {
    status = kTransitionErrorNullPointer;
    return false;
  }

  LabelType state_name("");
  int state_total_count = 0;
  LabelModelType label_model;

  char buff[4096] = {0,};

  while (NULL != fgets(buff, 4096, fin)) {
    std::string line(buff);
    line = hmmutil::Trim(line);
    if (line == observation_open_tag_) {
      obs_flag = FLAG_ON;
      continue;
    }
    else if (line == observation_close_tag_) {
      obs_flag = FLAG_OFF;
      break;
    }
    if (!obs_flag)
      continue;
    if (hmmutil::StartsWith(line, state_open_tag_)) {
      std::vector<std::string> tokens = hmmutil::SplitToken(line, count_delimiter_);
      if (tokens.size() != 3 || !hmmutil::StringToInt(tokens.at(2), state_total_count)) {
        status = kObservationErrorFileFormatError;
        Clear();
        return false;
      }
      state_name = tokens.at(1);
      state_flag = FLAG_ON;
    }
    else if (hmmutil::StartsWith(line, state_close_tag_)) {
      model_.insert(std::make_pair(state_name, std::make_pair(state_total_count, label_model)));
      state_total_count = 0;
      label_model.clear();
      state_name = "";
      state_flag = FLAG_OFF;
    }
    else {
      if (!state_flag)
        continue;
      std::vector<std::string> tokens = hmmutil::SplitToken(line, count_delimiter_);
      int count;
      if (tokens.size() != 2 || !hmmutil::StringToInt(tokens.at(1), count)) {
        status = kObservationErrorFileFormatError;
        Clear();
        return false;
      }
      label_model.insert(std::make_pair(tokens.at(0), count));
    }
  }
  if (state_flag || obs_flag) {
    status = kObservationErrorFileFormatError;
    Clear();
    return false;
  }

  int max_count = 0;
  ModelType::iterator it_model = model_.begin();
  for (; it_model != model_.end(); it_model++) {
    if (it_model->second.first > max_count)
      max_count = it_model->second.first;
  }
  oov_score_ = log(0.5/(double)max_count);

  return true;
}
bool ObservationModel::WriteModel(std::string model_path, HMMErrorType &status) {
  FILE* fout = fopen(model_path.c_str(), "w");
  if (!fout) {
    status = kObservationErrorFileOpenFail;
    return false;
  }
  return WriteModel(fout, status);
}
bool ObservationModel::WriteModel(FILE* fout, HMMErrorType &status) {
  fprintf(fout, "%s\n", observation_open_tag_.c_str());
  ModelType::iterator it_model = model_.begin();
  for (; it_model != model_.end(); it_model++) {
    fprintf(fout, "%s\t%s\t%s\n", state_open_tag_.c_str(), it_model->first.c_str(), hmmutil::IntToString(it_model->second.first).c_str());
    LabelModelType::iterator it_state = it_model->second.second.begin();
    for (; it_state != it_model->second.second.end(); it_state++) {
      fprintf(fout, "%s\t%s\n", it_state->first.c_str(), hmmutil::IntToString(it_state->second).c_str());
    }
    fprintf(fout, "%s\n", state_close_tag_.c_str());
  }
  fprintf(fout, "%s\n", observation_close_tag_.c_str());
  return true;
}
bool ObservationModel::Learn(TrainingSetParser& parser, HMMErrorType &status) {
  ParserBlockIterator it_block(parser);
  for (; !it_block.Done(); it_block.Next()) {
    ParserItemIterator it_item(it_block);
    for (; !it_item.Done(); it_item.Next()) {
      OutputType item_output = it_item.Value().first;
      LabelType item_state = it_item.Value().second;
      if (model_.find(item_state) == model_.end()) {
        LabelModelType label_model;
        label_model.insert(std::make_pair(item_output, 1));
        model_.insert(std::make_pair(item_state, std::make_pair(1, label_model)));
      }
      else {
        if (model_.at(item_state).second.find(item_output) == model_.at(item_state).second.end()) 
          model_.at(item_state).second.insert(std::make_pair(item_output, 1));
        else
          model_.at(item_state).second.at(item_output)++;
        model_.at(item_state).first++;
      }
    }
  }
  
  int max_count = 0;
  ModelType::iterator it_model = model_.begin();
  for (; it_model != model_.end(); it_model++) {
    if (it_model->second.first > max_count)
      max_count = it_model->second.first;
  }
  oov_score_ = log(0.5/(double)max_count);

  return true;
}

ScoreType ObservationModel::GetScore(LabelType state, OutputType output, HMMErrorType &status) {
  status = kObservationSuccess;
  
  ModelType::iterator it_model = model_.find(state);
  if (it_model == model_.end()) {
    status = kObservationErrorUnknownState;
    return log(0.0);
  }

  ScoreType total_count = (double)it_model->second.first;
  LabelModelType::iterator it_statemodel = it_model->second.second.find(output);
  if (it_statemodel == it_model->second.second.end()) {
    return oov_score_;
  }
  return log(((ScoreType)it_statemodel->second/total_count));
}

ScoreType ObservationModel::GetOOVScore() {
  return oov_score_;
}

void ObservationModel::Clear() {
  model_.clear();
}

} // namespace hmm