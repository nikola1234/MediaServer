#include <fstream>
#include <iostream>
#include <string>
#include <iconv.h>

using namespace std;

class CodeConverter
{
private:
    static size_t AToB(const char *from, const char *to, char *source, size_t source_len,
                char **target, size_t target_len);
    static int GetUtf8Array(ifstream &source, char **utf8, size_t n);                
public:
    static size_t GB2312ToUnicode(char *source, size_t source_len, short **target);
    static size_t UnicodeToGB2312(short *source, size_t source_len, char **target);
    static size_t UnicodeToUtf8(short *source, size_t source_len, char **target);
    static size_t Utf8ToUnicode(char *source, size_t source_len, short **target);
    static size_t GB2312ToUtf8(char *source, size_t source_len, char **target);
    static size_t Utf8ToGB2312(char *source, size_t source_len, char **target);
    static size_t Utf8ToGB2312(char *source, char *target);
};
