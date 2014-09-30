#ifndef HMM_COMMON_TYPEDEF_H_
#define HMM_COMMON_TYPEDEF_H_

#include <string>

namespace hmm {

#define FLAG_ON true
#define FLAG_OFF false

typedef bool HMM_FLAG;
typedef std::string LabelType;
typedef std::string OutputType;

typedef float ScoreType;

enum HMMErrorType {
  kSuccess = 0,
  kErrorFail,
  kLearnSuccess = 200,
  kParserSuccess = 250,
  kParserWarningHasLackEntry,
  kParserWarningMax,
  kObservationSuccess = 300,
  kObservationWarningMax,
  kObservationErrorFileOpenFail,
  kObservationErrorFileFormatError,
  kObservationErrorUnknownState,
  kTransitionSuccess = 400,
  kTransitionErrorNullPointer,
  kTransitionWarningMax,
  kTransitionErrorFileOpenFail,
  kTransitionErrorFileFormatError,
  kTransitionErrorUnknownState,
};

} // namespace hmm


#endif