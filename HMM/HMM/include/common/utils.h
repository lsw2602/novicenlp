#ifndef HMM_COMMON_UTILS_H_
#define HMM_COMMON_UTILS_H_ 1

#include <string>
#include <vector>

namespace hmmutil {
typedef enum UNICODE_CONV_STATE {
  UNI_CONV_ERR_NONE = 0,
  UNI_CONV_ERR_NOT_UTF8_FORMAT,
  UNI_CONV_WARN_BUFF_OVERFLOW,
  UNI_CONV_WARN_INPUTEMPTY,
  UNI_CONV_STATE_MAX
} UNICODE_CONV_STATE;

std::string OcrlmUtilUnicodeToUtf8(wchar_t unicode);
int UnicodeToUtf8(const wchar_t* uni_str, int uni_length, char* utf8_buff, int buff_length, UNICODE_CONV_STATE* state);
int Utf8ToUnicode(const char* utf8_str, int utf8_length, wchar_t* uni_buff, int buff_length, UNICODE_CONV_STATE* state);
std::vector<std::string> SplitToken(std::string s, std::string del);
std::string RightTrim( const std::string& s, const std::string& delimiters = " \f\n\r\t\v");
std::string LeftTrim( const std::string& s, const std::string& delimiters = " \f\n\r\t\v");
std::string Trim( const std::string& s, const std::string& delimiters = " \f\n\r\t\v");
bool StartsWith(std::string str, std::string startStr);
bool StringToInt(std::string str, int &num);
bool StringToFloat(std::string str, double &num);
std::string IntToString(int num);
}	//namespace ocrlmutil

#ifdef _WIN32
//#include <regex>
#include <windows.h>
#endif

#ifdef UNICODE
#define LMT_GET_FILENAMES LMT_get_filenames_w
int LMT_get_filenames_w(wchar_t* path,std::vector<std::wstring> &vt_filenames);
#else
#define LMT_GET_FILENAMES LMT_get_filenames_a
int LMT_get_filenames_a(char* path,std::vector<std::string> &vt_filenames);
#endif


#endif