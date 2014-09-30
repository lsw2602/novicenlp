#ifndef HMM_LEARNHMM_PARSER_TEXT_TRAININGSETPARSER_H_
#define HMM_LEARNHMM_PARSER_TEXT_TRAININGSETPARSER_H_

#include "trainingset_parser.h"

namespace hmm {

class TextTrainingSetParser :
  public TrainingSetParser
{
public:
  TextTrainingSetParser(void);
  ~TextTrainingSetParser(void);

public:
  void SetDelimiter(std::map<eDelimiter,std::string> dels);
  bool Parse(std::string filepath, HMMErrorType &state);
  bool Parse(FILE* fin, HMMErrorType &state);
  void Clear(void);

private:
  std::string begin_seq_tag_;
  std::string end_seq_tag_;
  std::string token_delimiter_;
  std::string transition_delimiter_;
};

} //namespace hmm

#endif