#ifndef HMM_LEARNHMM_MODEL_TRANSITIONMODEL_H_
#define HMM_LEARNHMM_MODEL_TRANSITIONMODEL_H_


#include <stdio.h>
#include <string>
#include <map>
#include <set>
#include <list>

#include "common/typedef.h"

namespace hmm {

class TrainingSetParser;
class TransitionNode;

class TransitionModel {
public:
  TransitionModel();
  ~TransitionModel();
  
  bool ReadModel(std::string model_path, HMMErrorType &status);
  bool ReadModel(FILE* fin, HMMErrorType &status);
  bool WriteModel(std::string model_path, HMMErrorType &status);
  bool WriteModel(FILE* fout, HMMErrorType &status);
  bool Learn(TrainingSetParser& parser, HMMErrorType &status);

  ScoreType GetScore(LabelType s1, LabelType s2, LabelType s3, HMMErrorType &status);

  std::set<std::string> get_state_set(void);

  void Clear();

public:
  static const std::string transition_open_tag_;
  static const std::string transition_close_tag_;
  static const std::string ngram_field_tag_;

private:
  TransitionNode* model_;
  std::set<std::string> state_set_;
  double lambda1_;
  double lambda2_;
  double lambda3_;
  int uni_count_;
};

class TransitionNode {
public:
  TransitionNode(std::string state, int count) {
    state_ = state;
    count_ = count;
  }
  ~TransitionNode() {
    std::map<std::string, TransitionNode*>::iterator it;
    for (it=children_.begin(); it != children_.end(); it++) {
      delete it->second;
    }
  }
  int get_count() {
    return count_;
  }
  void set_count(int count) {
    count_ = count;
  }
  bool InsertNode(std::list<std::string> state_seq, int count, bool is_read=false) {
    if (state_seq.empty()) {
      count_ += count;
      return true;
    }
    std::list<std::string>::iterator it = state_seq.begin();
    std::string state = *it;
    TransitionNode* child_node;
    if (children_.find(state) == children_.end()) {
      child_node = new TransitionNode(state, 0);
      children_.insert(std::make_pair(state, child_node));
    }
    else {
      child_node = children_.find(state)->second;
    }
    state_seq.pop_front();
    if (!is_read)
      count_ += count;
    return child_node->InsertNode(state_seq, count, is_read);
  }
  bool InsertNode(std::list<std::string> state_seq) {
    if (state_seq.empty()) {
      count_++;
      return true;
    }
    std::list<std::string>::iterator it = state_seq.begin();
    std::string state = *it;
    TransitionNode* child_node;
    if (children_.find(state) == children_.end()) {
      child_node = new TransitionNode(state, 0);
      children_.insert(std::make_pair(state, child_node));
    }
    else {
      child_node = children_.find(state)->second;
    }
    state_seq.pop_front();
    count_++;
    return child_node->InsertNode(state_seq);
  }
  bool WriteNode(FILE* fout, int depth, std::string state="", int cur_depth=0) {
    if (cur_depth == depth) {
      fprintf(fout, "%s\t%d\n", state.c_str(), count_);
      return true;
    }
    std::map<std::string,TransitionNode*>::iterator it;
    for (it=children_.begin(); it != children_.end(); it++) {
      if (!state.empty())
        it->second->WriteNode(fout, depth, state + " " + it->first, cur_depth+1);
      else
        it->second->WriteNode(fout, depth, it->first, cur_depth+1);
    }
    return true;
  }
  int GetCount(std::string s1, std::string s2, std::string s3) {
    std::list<std::string> state_seq;
    state_seq.push_back(s1);
    state_seq.push_back(s2);
    state_seq.push_back(s3);
    return GetCount(state_seq);
  }
  int GetCount(std::string s1, std::string s2) {
    std::list<std::string> state_seq;
    state_seq.push_back(s1);
    state_seq.push_back(s2);
    return GetCount(state_seq);
  }
  int GetCount(std::string s1) {
    std::list<std::string> state_seq;
    state_seq.push_back(s1);
    return GetCount(state_seq);
  }
  int GetCount(std::list<std::string> state_seq) {
    if (state_seq.empty()) {
      return count_;
    }
    std::list<std::string>::iterator it = state_seq.begin();
    std::string state = *it;
    TransitionNode* child_node;
    if (children_.find(state) == children_.end()) {
      return 0;
    }
    else {
      child_node = children_.find(state)->second;
    }
    state_seq.pop_front();
    return child_node->GetCount(state_seq);
  }

private:
  std::string state_;
  std::map<std::string, TransitionNode*> children_;
  int count_;
};

} // namespace hmm

#endif