#include "learnhmm\learning\hmm_learning_machine.h"

#include <list>
#include <set>
#include <cfloat>

#include "learnhmm\parser\text_trainingset_parser.h"
#include "learnhmm\model\observation_model.h"
#include "learnhmm\model\transition_model.h"
#include "common\utils.h"

namespace hmm {

BasicHMM::BasicHMM() : parser_(NULL), observ_model_(NULL), trans_model_(NULL) {

}

BasicHMM::~BasicHMM() {

}

bool BasicHMM::Init() {
  parser_ = new TextTrainingSetParser();
  observ_model_ = new ObservationModel();
  trans_model_ = new TransitionModel();
  if ( parser_ && observ_model_ && trans_model_)
    return true;
  else
    return false;
}

bool BasicHMM::Learn(std::string filepath, HMMErrorType &status) {
  parser_->Clear();

  parser_->Parse(filepath, status);
  if (status > kParserWarningMax)
    return false;

  observ_model_->Learn(*parser_, status);
  if (status > kObservationWarningMax)
    return false;

  trans_model_->Learn(*parser_, status);
  if (status > kTransitionWarningMax)
    return false;

  status = kSuccess;

  return true;
}

bool BasicHMM::Read(std::string db_path, HMMErrorType &status) {
  status = kSuccess;
  std::string observation_model_path = db_path + "/observation.txt";
  std::string transition_model_path = db_path + "/transition.txt";
  if (!observ_model_->ReadModel(observation_model_path, status)) {
    printf("observation model reading fail :: error code %d", status);
    return false;
  }
  if (!trans_model_->ReadModel(transition_model_path, status)) {
    printf("transition model reading fail :: error code %d", status);
    return false;
  }
  return true;
}

bool BasicHMM::Write(std::string outdir, HMMErrorType &status) {
  status = kSuccess;
  std::string observ_filepath = outdir + "/observation.txt";
  observ_model_->WriteModel(observ_filepath, status);
  if (status > kObservationWarningMax)
    return false;

  std::string trans_filepath = outdir + "/transition.txt";
  trans_model_->WriteModel(trans_filepath, status);
  if (status > kTransitionWarningMax)
    return false;

  status = kSuccess;

  return true;
}

std::list<LabelType> BasicHMM::Tagger(std::vector<OutputType> output_seq, HMMErrorType &status) {
  int output_seq_size = output_seq.size();
  std::list<LabelType> label_seq;

  std::vector<std::vector<HmmNode*>> hmm_table;

  for (int i=-2; i <= output_seq_size; i++) {
    hmm_table.push_back(GetSet(i, output_seq_size));
  }
  hmm_table.at(1).at(0)->set_prev_node(hmm_table.at(0).at(0));
  
  for (int i=0; i < output_seq_size; i++) {
    std::vector<HmmNode*>::iterator it_cur;
    for (it_cur = hmm_table.at(i+2).begin(); it_cur != hmm_table.at(i+2).end(); it_cur++) {
      HmmNode* cur_node = *it_cur;
      std::vector<HmmNode*>::iterator it_prev;
      for (it_prev = hmm_table.at(i+1).begin(); it_prev != hmm_table.at(i+1).end(); it_prev++) {
        HmmNode* prev_node = *it_prev;
        ScoreType score = GetScore(cur_node, prev_node, output_seq.at(i));
        if (score > cur_node->get_score()) {
          cur_node->set_score(score);
          cur_node->set_prev_node(prev_node);
        }
      }
    }
  }
       
  HmmNode* cur_node = hmm_table.at(output_seq_size+2).at(0);
  std::vector<HmmNode*>::iterator it_prev;
  for (it_prev = hmm_table.at(output_seq_size+1).begin(); it_prev != hmm_table.at(output_seq_size+1).end(); it_prev++) {
    HmmNode* prev_node = *it_prev;
    if (prev_node->get_prev_node() == NULL)
      continue;
    ScoreType score = prev_node->get_score() + trans_model_->GetScore(prev_node->get_prev_node()->get_state(), prev_node->get_state(), cur_node->get_state(), status); 
    if (score > cur_node->get_score()) {
      cur_node->set_score(score);
      cur_node->set_prev_node(prev_node);
    }
  }


  HmmNode* end_node = hmm_table.at(output_seq_size+2).at(0);

  while (end_node->get_prev_node() != NULL) {
    end_node = end_node->get_prev_node();
    if (end_node == NULL || end_node->get_state() == "<s>")
      break;
    label_seq.push_front(end_node->get_state());
  }

  //release hmm_table;
  std::vector<std::vector<HmmNode*>>::iterator it_hmm_table;
  for (it_hmm_table = hmm_table.begin(); it_hmm_table != hmm_table.end(); it_hmm_table++) {
    std::vector<HmmNode*>::iterator it_set = it_hmm_table->begin();
    for (; it_set != it_hmm_table->end(); it_set++) {
      delete *it_set;
    }
  }

  return label_seq;

}

//int total_count = 0;
//ReturnStruct BasicHMM::GetPi(int k, std::string u, std::string v, std::vector<std::string> &tokens) {
//  //printf("%d\n",total_count++);
//  printf("%d\n", pi_table_.size());
////  if (total_count == 5019)
////    int a = 0;
//  if (pi_table_.find(std::make_pair(k, u + " " + v)) != pi_table_.end())
//    return pi_table_.at(std::make_pair(k, u + " " + v));
//
//  hmm::HMMErrorType status;
//  
//  ReturnStruct best_pi;
//  best_pi.score= FLT_MIN;
//
//  std::set<std::string> w_set = GetSet(k-2);
//  std::set<std::string>::iterator it_w_set = w_set.begin();
//  for (; it_w_set != w_set.end(); it_w_set++) {
//    ReturnStruct prev_pi = GetPi(k-1, *it_w_set, u, tokens);
//    ScoreType observ_score;
//    if (k > tokens.size())
//      observ_score = log(1);
//    else
//      observ_score = observ_model_->GetScore(v, tokens.at(k-1), status);
//    ScoreType score = prev_pi.score + trans_model_->GetScore(*it_w_set, u, v, status) + observ_score;
//    if (score > best_pi.score) {
//      best_pi.score = score;
//      best_pi.labels = prev_pi.labels;
//      best_pi.labels.push_back(*it_w_set);
//    }
//  }
//  pi_table_.insert(std::make_pair(std::make_pair(k,u+" "+v), best_pi));
//  return best_pi;
//}

std::vector<HmmNode*> BasicHMM::GetSet(int k, int size) {
  std::vector<HmmNode*> k_set;
  if (k == -2 || k == -1) {
    k_set.push_back(new HmmNode(0.0, NULL, "<s>"));
  }
  else if (k == size) {
    k_set.push_back(new HmmNode(log(0.0), NULL, "<STOP>"));
  }
  else {
    std::set<LabelType> tmp_set = trans_model_->get_state_set();
    std::set<LabelType>::iterator it_tmp_set;
    for (it_tmp_set = tmp_set.begin(); it_tmp_set != tmp_set.end(); it_tmp_set++) {
      if (*it_tmp_set == "<s>" || *it_tmp_set == "<STOP>")
        continue;
      k_set.push_back(new HmmNode(log(0.0), NULL, *it_tmp_set));
    }
  }
  return k_set;
}

ScoreType BasicHMM::GetScore(HmmNode* cur_node, HmmNode* prev_node, std::string output) {
  HMMErrorType status;
  ScoreType obs_score = observ_model_->GetScore(cur_node->get_state(), output, status);
  ScoreType trs_score;
  if (prev_node->get_prev_node() != NULL)
    trs_score = trans_model_->GetScore(prev_node->get_prev_node()->get_state(), prev_node->get_state(), cur_node->get_state(), status);
  else
    trs_score = log(0.0);
  return prev_node->get_score() + obs_score + trs_score;
}

} // namespace hmm