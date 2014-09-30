#ifndef HMM_LEARNHMM_PARSER_TRAININGSETPARSER_H_
#define HMM_LEARNHMM_PARSER_TRAININGSETPARSER_H_

#include <map>
#include <vector>
#include <string>

namespace hmm {

enum HMMErrorType;

class TrainingSetParser {
  friend class ParserBlockIterator;
protected:
  enum eDelimiter {
    kBeginSequenceTag = 0,
    kEndSequenceTag,
    kTokenDelimiter,
    kTransitionDelimiter,
    kSequenceTag,
    kTokenTag,
    kTransitionTag,
    kObservationTag,
  };
public:
  TrainingSetParser(void);
  ~TrainingSetParser(void);

  virtual void SetDelimiter(std::map<eDelimiter,std::string> dels) = 0;
  virtual bool Parse(std::string filepath, HMMErrorType &state) = 0;
  virtual bool Parse(FILE* fin, HMMErrorType &state) = 0;
  virtual void Clear(void) = 0;

protected:
  std::vector<std::vector<std::pair<std::string,std::string>>> training_set_;

};

class ParserBlockIterator {
  friend class ParserItemIterator;
public:
  ParserBlockIterator(TrainingSetParser &parser) : parser_(parser) {
    it_block_ = parser_.training_set_.begin();
  };
  ~ParserBlockIterator() {};
      
  bool Done() {
    return it_block_ == parser_.training_set_.end();
  }

  void Next() {
    it_block_++;
  }

  std::vector<std::pair<std::string, std::string>> Value() {
    return *it_block_;
  }

private:
  std::vector<std::vector<std::pair<std::string, std::string>>>::iterator it_block_;
  TrainingSetParser& parser_;
};

class ParserItemIterator {
public:
  ParserItemIterator(ParserBlockIterator& block) : block_(block) {
    it_item_ = block_.it_block_->begin();
  };
  ~ParserItemIterator() {};
      
  bool Done() {
    return it_item_ == block_.it_block_->end();
  }

  void Next() {
    it_item_++;
  }

  std::pair<std::string, std::string> Value() {
    return *it_item_;
  }

private:
  std::vector<std::pair<std::string, std::string>>::iterator it_item_;
  ParserBlockIterator& block_;
};
} // namespace hmm

#endif