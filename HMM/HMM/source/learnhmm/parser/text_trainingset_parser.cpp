#include "learnhmm/parser/text_trainingset_parser.h"

#include <string>
#include <vector>

#include "common/utils.h"
#include "common/typedef.h"

namespace hmm {

TextTrainingSetParser::TextTrainingSetParser(void) : begin_seq_tag_("<BOS>"), end_seq_tag_("<EOS>"), token_delimiter_("\n"), transition_delimiter_("\t")
{
  printf("text parser constructer.\n");
}


TextTrainingSetParser::~TextTrainingSetParser(void)
{
  printf("text parser delete.\n");
}

void TextTrainingSetParser::SetDelimiter(std::map<eDelimiter,std::string> dels) {
  begin_seq_tag_ = dels.at(kBeginSequenceTag);
  end_seq_tag_ = dels.at(kEndSequenceTag);
  token_delimiter_ = dels.at(kTokenDelimiter);
  transition_delimiter_ = dels.at(kTransitionDelimiter);
}

bool TextTrainingSetParser::Parse(std::string filepath, HMMErrorType &state) {
  FILE* fin = fopen(filepath.c_str(), "r");
  return Parse(fin, state);
}

bool TextTrainingSetParser::Parse(FILE* fin, HMMErrorType &state) {
  state = kParserSuccess;
  std::vector<std::pair<std::string, std::string>> training_set_block;
  
  HMM_FLAG train_block_flag = FLAG_OFF;

  char buff[8096] = {0,};
  
  while (NULL != fgets(buff, 8096, fin)) {
    std::string line(buff);
    line = hmmutil::Trim(line);
    if (line.empty())
      continue;
    if (line == begin_seq_tag_) {
      train_block_flag = FLAG_ON;
      continue;
    }
    if (line == end_seq_tag_) {
      training_set_.push_back(training_set_block);
      training_set_block.clear();
      train_block_flag = FLAG_OFF;
      continue;
    }
    std::vector<std::string> tokens = hmmutil::SplitToken(line, transition_delimiter_);
    if (tokens.size() != 2) {
      state = kParserWarningHasLackEntry;
      continue;
    }
    
    training_set_block.push_back(std::make_pair(tokens.at(0), tokens.at(1)));
  }
  return true;
}

void TextTrainingSetParser::Clear() {
  training_set_.clear();
}
} // namespace hmm
