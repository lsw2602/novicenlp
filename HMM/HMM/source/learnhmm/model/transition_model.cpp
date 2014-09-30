#include "learnhmm/model/transition_model.h"

#include "learnhmm/parser/trainingset_parser.h"
#include "common/utils.h"

namespace hmm {
  
const std::string TransitionModel::transition_open_tag_   = "<transition_model>";
const std::string TransitionModel::transition_close_tag_  = "</transition_model>";
const std::string TransitionModel::ngram_field_tag_       = "<ngram n=#>";

TransitionModel::TransitionModel() {
  lambda1_ = 0.33;
  lambda2_ = 0.33;
  lambda3_ = 0.34;
  uni_count_ = 0;
  model_ = new TransitionNode("",0);
}

TransitionModel::~TransitionModel() {
}
  
bool TransitionModel::ReadModel(std::string model_path, HMMErrorType &status) {
  FILE* fin = fopen(model_path.c_str(), "r");
  if (!fin) {
    status = kObservationErrorFileOpenFail;
    return false;
  }
  return ReadModel(fin, status);
}
bool TransitionModel::ReadModel(FILE* fin, HMMErrorType &status) {
  status = kTransitionSuccess;
  int n_gram = 0;
  
  if (fin == NULL) {
    status = kTransitionErrorNullPointer;
    return false;
  }
  HMM_FLAG trs_flag = FLAG_OFF;
  
  std::map<std::string, int> state;

  char buff[4096] = {0,};

  while (NULL != fgets(buff, 4096, fin)) {
    std::string line(buff);
    line = hmmutil::Trim(line);
    if (line == transition_open_tag_) {
      trs_flag = FLAG_ON;
      continue;
    }
    else if (line == transition_close_tag_) {
      trs_flag = FLAG_OFF;
      break;
    }
    else {
      if (!trs_flag)
        continue;
      std::vector<std::string> tokens = hmmutil::SplitToken(line, "\t");
      if (tokens.size() != 2) {
        if (hmmutil::StartsWith(line, "<lambda")) {
          if (line.at(7) == '1') {
            hmmutil::StringToFloat(line.substr(10), lambda1_);
          }
          else if (line.at(7) == '2') {
            hmmutil::StringToFloat(line.substr(10), lambda2_);
          }
          else if (line.at(7) == '3') {
            hmmutil::StringToFloat(line.substr(10), lambda3_);
          }
          else
            continue;
        }
        else if (hmmutil::StartsWith(line, "<unicount>=")) {
          int uni_count;
          hmmutil::StringToInt(line.substr(11), uni_count);
          model_->set_count(uni_count);
        }
        else if (hmmutil::StartsWith(line, "<ngram n=")) {
          n_gram++;
        }
        continue;
      }
      std::list<std::string> state_seq;
      std::vector<std::string> vState_seq = hmmutil::SplitToken(tokens.at(0), " ");
      std::vector<std::string>::iterator it_vState = vState_seq.begin();
      for (; it_vState != vState_seq.end(); it_vState++) {
        state_seq.push_back(*it_vState);
      }
      int count;
      if (!hmmutil::StringToInt(tokens.at(1), count)) {
        status = kTransitionErrorFileFormatError;
        return false;
      }
      if (n_gram == 1)
        state_set_.insert(tokens.at(0));
      model_->InsertNode(state_seq, count, true);
    }
  }
  return true;
}
bool TransitionModel::WriteModel(std::string model_path, HMMErrorType &status) {
  FILE* fout = fopen(model_path.c_str(), "w");
  if (!fout) {
    status = kObservationErrorFileOpenFail;
    return false;
  }
  return WriteModel(fout, status);
}
bool TransitionModel::WriteModel(FILE* fout, HMMErrorType &status) {
  status = kTransitionSuccess;
  
  if (fout == NULL) {
    status = kTransitionErrorNullPointer;
    return false;
  }
  
  fprintf(fout, "%s\n", transition_open_tag_.c_str());
  
  fprintf(fout, "<unicount>=%d\n", model_->get_count());
  fprintf(fout, "<lambda1>=%.5f\n", lambda1_);
  fprintf(fout, "<lambda2>=%.5f\n", lambda2_);
  fprintf(fout, "<lambda3>=%.5f\n", lambda3_);

  std::vector<std::string>::iterator it;
  std::string ngram_field_tag = ngram_field_tag_;
  ngram_field_tag.replace(ngram_field_tag.find("#"), 1, "1");
  fprintf(fout, "%s\n", ngram_field_tag.c_str());

  model_->WriteNode(fout, 1);

  ngram_field_tag = ngram_field_tag_;
  ngram_field_tag.replace(ngram_field_tag.find("#"), 1, "2");
  fprintf(fout, "%s\n", ngram_field_tag.c_str());

  model_->WriteNode(fout, 2);
  
  ngram_field_tag = ngram_field_tag_;
  ngram_field_tag.replace(ngram_field_tag.find("#"), 1, "3");
  fprintf(fout, "%s\n", ngram_field_tag.c_str());
  
  model_->WriteNode(fout, 3);

  fprintf(fout, "%s\n", transition_close_tag_.c_str());
  
  return true;
}
bool TransitionModel::Learn(TrainingSetParser& parser, HMMErrorType &status) {
  ParserBlockIterator it_block(parser);
  for (; !it_block.Done(); it_block.Next()) {
    ParserItemIterator it_item(it_block);
    std::string second_prev_state = "<s>";
    std::string prev_state = "<s>";
    std::string cur_state = "<s>";
    for (; !it_item.Done(); it_item.Next()) {
      cur_state = it_item.Value().second;
      state_set_.insert(cur_state);
      std::list<std::string> state_seq;
      state_seq.push_back(second_prev_state);
      state_seq.push_back(prev_state);
      state_seq.push_back(cur_state);
      model_->InsertNode(state_seq);
      second_prev_state = prev_state;
      prev_state = cur_state;
    }
    std::list<std::string> state_seq;
    cur_state = "<STOP>";
    state_seq.push_back(second_prev_state);
    state_seq.push_back(prev_state);
    state_seq.push_back(cur_state);
    model_->InsertNode(state_seq);
    state_seq.pop_front();
    model_->InsertNode(state_seq);
    state_seq.pop_front();
    model_->InsertNode(state_seq);
  }
  return true;
}

std::set<std::string> TransitionModel::get_state_set(void) {
  return state_set_;
}

ScoreType TransitionModel::GetScore(std::string s1, std::string s2, std::string s3, HMMErrorType &status) {
  ScoreType score = 0.0;
  if (model_->get_count() != 0)
    score += lambda1_ * (ScoreType)model_->GetCount(s3)/(ScoreType)model_->get_count();
  if (model_->GetCount(s2) != 0) 
    score += lambda2_ * (ScoreType)model_->GetCount(s2, s3) /(ScoreType)model_->GetCount(s2);
  if (model_->GetCount(s1,s2) != 0) 
    score += lambda3_ * (ScoreType)model_->GetCount(s1, s2, s3) / (ScoreType)model_->GetCount(s1,s2);
  return log(score);
}

void TransitionModel::Clear() {
  if (model_)
    delete model_;
}

} // namespace hmm