#include "common/utils.h"

namespace hmmutil {
std::string OcrlmUtilUnicodeToUtf8(wchar_t unicode) {
  char utf8[4] = {0,};
  if (unicode & 0xF800) {
    utf8[2] = (char)((0x003F & unicode) | 0x0080);
    utf8[1] = (char)((0x003F & (unicode >>= 6)) | 0x0080);
    utf8[0] = (char)((0x000F & (unicode >> 6)) | 0x00E0);
  }
  else if (unicode & 0x0780) {
    utf8[1] = (char)((0x003F & unicode) | 0x0080);
    utf8[0] = (char)((0x001F & (unicode >> 6)) | 0x00C0);
  }
  else {
    utf8[0] = (char)(0x007F & unicode);
  }
  return std::string(utf8);
}

int UnicodeToUtf8(const wchar_t* uni_str, int uni_length, char* utf8_buff, int buff_length, UNICODE_CONV_STATE* state) {
  wchar_t* p;
  wchar_t unicode;
  char utf8[4] = {0,};
  int buff_point = 0;

  *state = UNI_CONV_ERR_NONE;
  memset(utf8_buff, 0, buff_length);
  for (p = (wchar_t*)uni_str; *p != 0x0000; p++) {
    unicode = *p;
    memset(utf8, 0, 4);
    if (unicode & 0xF800) {
      if ((buff_point+=3) >= buff_length) {
        *state = UNI_CONV_WARN_BUFF_OVERFLOW;
        return 1;
      }
      utf8[2] = (char)((0x003F & unicode) | 0x0080);
      utf8[1] = (char)((0x003F & (unicode >>= 6)) | 0x0080);
      utf8[0] = (char)((0x000F & (unicode >> 6)) | 0x00E0);
    }
    else if (*p & 0x0780) {
      if ((buff_point+=2) >= buff_length) {
        *state = UNI_CONV_WARN_BUFF_OVERFLOW;
        return 1;
      }
      utf8[1] = (char)((0x003F & unicode) | 0x0080);
      utf8[0] = (char)((0x001F & (unicode >> 6)) | 0x00C0);
    }
    else {
      if ((buff_point+=1) >= buff_length) {
        *state = UNI_CONV_WARN_BUFF_OVERFLOW;
        return 1;
      }
      utf8[0] = (char)(0x007F & unicode);
    }
    strcat(utf8_buff, utf8);
  }
  return 0;
}

int Utf8ToUnicode(const char* utf8_str, int utf8_length, wchar_t* uni_buff, int buff_length, UNICODE_CONV_STATE* state){
  char* p;
  wchar_t unicode;
  int buff_point = 0, i;

  memset(uni_buff,0,buff_length*sizeof(wchar_t));
  *state = UNI_CONV_ERR_NONE;
  for (p=(char*)utf8_str; *p != NULL; p++) {
    if (*p & 0x80) {
      if ((*p & 0xE0) == 0xC0) {
        unicode = ((0x0000 | (*p & 0x1F)) << 6);
        if ((*(++p) & 0xC0) != 0x80) {
          *state = UNI_CONV_ERR_NOT_UTF8_FORMAT;
          uni_buff[0] = 0x0000;
          return -1;
        }
        unicode |= (*p &0x3F);
        if (buff_point >= buff_length) {
          *state = UNI_CONV_WARN_BUFF_OVERFLOW;
          return 1;
        }
        uni_buff[buff_point++] = unicode;
      }
      else if ((*p & 0xF0) == 0xE0) {
        unicode = ((0x0000 | (*p & 0x0F)) << 12);
        for (i=1; i<=2; i++) {
          if ((*(++p) & 0xC0) != 0x80) {
            *state = UNI_CONV_ERR_NOT_UTF8_FORMAT;
            uni_buff[0] = 0x0000;
            return -1;
          }
          unicode |= ((*p &0x3F) <<  6*(2-i));
        }
        if (buff_point >= buff_length) {
          *state = UNI_CONV_WARN_BUFF_OVERFLOW;
          return 1;
        }
        uni_buff[buff_point++] = unicode;
      }
      else if ((*p & 0xF8) == 0xF0) {
        unicode = ((0x0000 | (*p & 0x07)) << 18);
        for (i=1; i<=3; i++) {
          if ((*(++p) & 0xC0) != 0x80) {
            *state = UNI_CONV_ERR_NOT_UTF8_FORMAT;
            uni_buff[0] = 0x0000;
            return -1;
          }
          unicode |= ((*p &0x3F) <<  6*(3-i));
        }
        if (buff_point >= buff_length) {
          *state = UNI_CONV_WARN_BUFF_OVERFLOW;
          return 1;
        }
        uni_buff[buff_point++] = unicode;
      }
    }
    else {
      if (buff_point >= buff_length) {
        *state = UNI_CONV_WARN_BUFF_OVERFLOW;
        return 1;
      }
      uni_buff[buff_point++] = (wchar_t)*p;
    }
  }
  return 0;
}

std::vector<std::string> SplitToken(std::string s, std::string del) {
	std::vector<std::string> ret;
	int pos;
	
	while( std::string::npos != (pos = s.find(del))) {
		ret.push_back(s.substr(0, pos));
		s = s.substr(pos + del.length());
	}
	ret.push_back(s);

	return ret;
}


std::string RightTrim(
  const std::string& s,
  const std::string& delimiters)
{
	size_t pos = s.find_last_not_of( delimiters );
	if (pos == std::string::npos)
		return std::string("");
  return s.substr( 0, pos+1 );
}

std::string LeftTrim(
  const std::string& s,
  const std::string& delimiters)
{
	size_t pos = s.find_first_not_of( delimiters );
	if (pos == std::string::npos)
		return std::string("");
  return s.substr( pos );
}

std::string Trim(
  const std::string& s,
  const std::string& delimiters)
{
  return LeftTrim( RightTrim( s, delimiters ), delimiters );
}

bool StartsWith(std::string str, std::string startStr) {
	if (str.length() < startStr.length())
		return false;
	for (int i=0; i < (int)startStr.length(); i++) {
		if (str[i] != startStr[i])
			return false;
	}
	return true;
}

bool StringToInt(std::string str, int &num) {
  num = 0;
  bool isMinus = false;
  int len = str.length();
  int i = 0;
  if (str[0] == '-') {
    isMinus = true;
    i++;
  }
  for (;i < len; i++) {
    if ('0' <= str[i] && str[i] <= '9') {
      num = num*10 + (str[i] - '0');
    }
    else
      return false;
  }
  if (isMinus)
    num = 0-num;
  return true;
}

bool StringToFloat(std::string str, double &num) {
  int dot_pos = str.find(".");
  bool has_dot = true;
  if (dot_pos == str.npos) {
    has_dot = false;
  }
  std::string s_natural = str.substr(0,dot_pos);
  int natural;
  double d_nump = 0.0;
  if (!StringToInt(s_natural, natural))
    return false;
  if (has_dot) {
    std::string s_nump = str.substr(dot_pos+1);
    int nump;
    if (!StringToInt(s_nump, nump))
      return false;
    d_nump = nump;
    for (int i=0; i < s_nump.length(); i++) {
      d_nump /= 10.0;
    }
  }
  num = (double)natural+d_nump;
  return true;
}

std::string IntToString(int num) {
	std::string str = "";
	if (num == 0) {
		return "0";
	}
	str = "";
	if (num < 0) {
		str += "-";
		num = 0-num;
	}
	std::string ret = "";
	for (;num>0;num /=10) {
		std::string tmp;
		tmp = '0' + num%10;
		ret = tmp + ret;
	}
	str += ret;
	return str;
}

} // namespace ocrlmutil

#ifdef _WIN32
#ifndef UNICODE
int LMT_get_filenames_a(char* path,std::vector<std::string> &vt_filenames){
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];

	//Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", path);

	if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		printf("Path not found: [%s]\n", path);
		return -1;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories.
		if(strcmp(fdFile.cFileName, ".") != 0
			&& strcmp(fdFile.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [sDir] and the file/foldername we just found:
			sprintf(sPath, "%s\\%s", path, fdFile.cFileName);

			//Is the entity a File or Folder?
			if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				LMT_get_filenames_a(sPath,vt_filenames); //Recursion, I love it!
			}
			else{
				vt_filenames.push_back(sPath);
			}
		}
	}
	while(FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return 0;
}
#else
int LMT_get_filenames_w(wchar_t* path,std::vector<std::wstring> &vt_filenames){
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[2048];

	//Specify a file mask. *.* = We want everything!
	wsprintf(sPath, L"%s\\*.*", path);

	if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		printf("Path not found: [%s]\n", path);
		return -1;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories.
		if(wcscmp(fdFile.cFileName, L".") != 0
			&& wcscmp(fdFile.cFileName, L"..") != 0)
		{
			//Build up our file path using the passed in
			//  [sDir] and the file/foldername we just found:
			wsprintf(sPath, L"%s\\%s", path, fdFile.cFileName);

			//Is the entity a File or Folder?
			if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				LMT_get_filenames_w(sPath,vt_filenames); //Recursion, I love it!
			}
			else{
				vt_filenames.push_back(sPath);
			}
		}
	}
	while(FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return 0;
}
#endif
#endif