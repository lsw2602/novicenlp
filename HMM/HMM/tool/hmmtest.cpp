#include "learnhmm\learning\hmm_learning_machine.h"

#include <iostream>
#include <vector>

#include "common\utils.h"

using namespace hmm;

int main(void) {
  HMMErrorType status;
  BasicHMM hmm = BasicHMM();
  std::string filepath("./data/test");
  //std::string filepath("./test");
  hmm.Init();
  //std::vector<std::string> files;
  //LMT_get_filenames_a((char*)filepath.c_str(), files);
  //
  //std::vector<std::string>::iterator it_files = files.begin();
  //for (; it_files != files.end(); it_files++) {
  //  printf("FILE : %s\n", it_files->c_str());
  //  hmm.Learn(*it_files, status);
  //}
  //hmm.Write("./", status);
  hmm.Read("./", status);
  hmm.Write("./model_test", status);
  std::vector<OutputType> input = hmmutil::SplitToken("He is so mean . What do you mean ? I just kicked the can .", " ");
  std::list<LabelType> result = hmm.Tagger(input, status);

  std::list<LabelType>::iterator it_result = result.begin();
  int idx = 0;
  for (; it_result != result.end(); it_result++, idx++) {
    std::cout << input.at(idx) + "\t" + *it_result << std::endl;
  }

  return 0;
}