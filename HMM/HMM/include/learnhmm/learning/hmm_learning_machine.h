#ifndef HMM_HMM_H_
#define HMM_HMM_H_

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>

#include "common\typedef.h"

namespace hmm {
  struct ReturnStruct {
    ScoreType score;
    std::list<LabelType> labels;
  };
  class TrainingSetParser;
  class ObservationModel;
  class TransitionModel;
  class HmmNode;
class BasicHMM {
public:
  BasicHMM();
  ~BasicHMM();

  bool Init(void);
  bool Learn(std::string filepath, HMMErrorType &status);
  bool Write(std::string outdir, HMMErrorType &status);
  bool Read(std::string db_path, HMMErrorType &status);
  std::list<std::string> Tagger(std::vector<std::string> output_seg, HMMErrorType &status);
private:
  TrainingSetParser* parser_;
  ObservationModel* observ_model_;
  TransitionModel* trans_model_;
  //std::map<std::pair<int, std::string>, ReturnStruct> pi_table_;

  ScoreType GetScore(HmmNode* cur_node, HmmNode* prev_node, std::string output);
  //ReturnStruct GetPi(int k, std::string u, std::string v, std::vector<std::string> &tokens);
  std::vector<HmmNode*> GetSet(int k, int size);
};

class HmmNode {
public:
  HmmNode() {
    prev_node_ = NULL;
    score_ = 0.0;
    state_ = "*";
  }
  HmmNode(ScoreType score, HmmNode* prev_node, std::string state) {
    prev_node_ = prev_node;
    score_ = score;
    state_ = state;
  }
  
  void set_state(LabelType state) {
    state_ = state;
  }
  LabelType get_state(void) {
    return state_;
  }
  void set_score(ScoreType score) {
    score_ = score;
  }
  ScoreType get_score(void) {
    return score_;
  }
  void set_prev_node(HmmNode* prev_node) {
    prev_node_ = prev_node;
  }
  HmmNode* get_prev_node(void) {
    return prev_node_;
  }
private:
  LabelType state_;
  HmmNode* prev_node_;
  ScoreType score_;
};

} // namespace hmm

#endif