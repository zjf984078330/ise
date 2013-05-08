/****************************************************************************\
*                                                                            *
*  ISE (Iris Server Engine) Project                                          *
*  http://github.com/haoxingeng/ise                                          *
*                                                                            *
*  Copyright 2013 HaoXinGeng (haoxingeng@gmail.com)                          *
*  All rights reserved.                                                      *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
\****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// ise_sys_utils.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_SYS_UTILS_H_
#define _ISE_SYS_UTILS_H_

#include "ise/main/ise_options.h"

#ifdef ISE_WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <windows.h>
#endif

#ifdef ISE_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#endif

#include "ise/main/ise_global_defs.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// 提前声明

class Buffer;
class StrList;

///////////////////////////////////////////////////////////////////////////////
// 类型定义

// 文件查找记录
struct FileFindItem
{
    INT64 fileSize;         // 文件大小
    std::string fileName;        // 文件名(不含路径)
    UINT attr;              // 文件属性
};

typedef std::vector<FileFindItem> FileFindResult;

///////////////////////////////////////////////////////////////////////////////
// 杂项函数

//-----------------------------------------------------------------------------
//-- 字符串函数:

bool isIntStr(const std::string& str);
bool isInt64Str(const std::string& str);
bool isFloatStr(const std::string& str);
bool isBoolStr(const std::string& str);

int strToInt(const std::string& str, int defaultVal = 0);
INT64 strToInt64(const std::string& str, INT64 defaultVal = 0);
std::string intToStr(int value);
std::string intToStr(INT64 value);
double strToFloat(const std::string& str, double defaultVal = 0);
std::string floatToStr(double value, const char *format = "%f");
bool strToBool(const std::string& str, bool defaultVal = false);
std::string boolToStr(bool value, bool useBoolStrs = false);

void formatStringV(std::string& result, const char *format, va_list argList);
std::string formatString(const char *format, ...);

bool sameText(const std::string& str1, const std::string& str2);
int compareText(const char* str1, const char* str2);
std::string trimString(const std::string& str);
std::string upperCase(const std::string& str);
std::string lowerCase(const std::string& str);
std::string repalceString(const std::string& sourceStr, const std::string& oldPattern,
    const std::string& newPattern, bool replaceAll = false, bool caseSensitive = true);
void splitString(const std::string& sourceStr, char splitter, StrList& strList,
    bool trimResult = false);
void splitStringToInt(const std::string& sourceStr, char splitter, IntegerArray& intList);
char* strNCopy(char *dest, const char *source, int maxBytes);
char* strNZCopy(char *dest, const char *source, int destSize);
std::string fetchStr(std::string& inputStr, char delimiter = ' ', bool del = true);
std::string addThousandSep(const INT64& number);

std::string getQuotedStr(const char* str, char quoteChar = '"');
std::string extractQuotedStr(const char*& str, char quoteChar = '"');
std::string getDequotedStr(const char* str, char quoteChar = '"');

//-----------------------------------------------------------------------------
//-- 文件和目录:

bool fileExists(const std::string& fileName);
bool directoryExists(const std::string& dir);
bool createDir(const std::string& dir);
bool deleteDir(const std::string& dir, bool recursive = false);
std::string extractFilePath(const std::string& fileName);
std::string extractFileName(const std::string& fileName);
std::string extractFileExt(const std::string& fileName);
std::string changeFileExt(const std::string& fileName, const std::string& ext);
bool forceDirectories(std::string dir);
bool deleteFile(const std::string& fileName);
bool removeFile(const std::string& fileName);
bool renameFile(const std::string& oldFileName, const std::string& newFileName);
INT64 getFileSize(const std::string& fileName);
void findFiles(const std::string& path, UINT attr, FileFindResult& findResult);
std::string pathWithSlash(const std::string& path);
std::string pathWithoutSlash(const std::string& path);
std::string GetAppExeName();
std::string getAppPath();
std::string getAppSubPath(const std::string& subDir = "");

//-----------------------------------------------------------------------------
//-- 系统相关:

int getLastSysError();
THREAD_ID getCurThreadId();
std::string sysErrorMessage(int errorCode);
void sleepSec(double seconds, bool allowInterrupt = true);
UINT64 getCurTicks();
UINT64 getTickDiff(UINT64 oldTicks, UINT64 newTicks);

//-----------------------------------------------------------------------------
//-- 其它函数:

void randomize();
int getRandom(int min, int max);
void generateRandomList(int startNumber, int endNumber, int *randomList);

template <typename T>
const T& min(const T& a, const T& b) { return ((a < b)? a : b); }

template <typename T>
const T& max(const T& a, const T& b) { return ((a < b)? b : a); }

template <typename T>
const T& ensureRange(const T& value, const T& minVal, const T& maxVal)
    { return (value > maxVal) ? maxVal : (value < minVal ? minVal : value); }

template <typename T>
void swap(T& a, T& b) { T temp(a); a=b; b=temp; }

template <typename T>
int compare(const T& a, const T& b) { return (a < b) ? -1 : (a > b ? 1 : 0); }

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_SYS_UTILS_H_
