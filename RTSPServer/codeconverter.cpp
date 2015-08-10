
#ifdef WIN32
#include "codeconverter.h"

size_t CodeConverter::GB2312ToUnicode(char *source, size_t source_len, short **target)
{
    //CP_ACP表示ASCII代码页。对于中文来讲，就是GB2312
	size_t target_len;
	target_len = MultiByteToWideChar(CP_ACP, 0, source, source_len, NULL, 0);
	*target = new short[target_len];
	return MultiByteToWideChar(CP_ACP, 0, source, source_len, (WCHAR*)*target, target_len);	
}
size_t CodeConverter::UnicodeToGB2312(short *source, size_t source_len, char **target)
{
	size_t target_len;
	target_len = WideCharToMultiByte(CP_ACP, 0, (WCHAR*)source, source_len, NULL, 0, NULL, NULL);	
	*target = new char[target_len];
    return WideCharToMultiByte(CP_ACP, 0, (WCHAR*)source, source_len, *target, target_len, NULL, NULL);	
}
size_t CodeConverter::UnicodeToUtf8(short *source, size_t source_len, char **target)
{
	size_t target_len;
	target_len = WideCharToMultiByte(CP_UTF8, 0, (WCHAR*)source, source_len, NULL, 0, NULL, NULL);	
	*target = new char[target_len];
    return WideCharToMultiByte(CP_UTF8, 0, (WCHAR*)source, source_len, *target, target_len, NULL, NULL);	
}
size_t CodeConverter::Utf8ToUnicode(char *source, size_t source_len, short **target)
{
	size_t target_len;
	target_len = MultiByteToWideChar(CP_UTF8, 0, source, source_len, NULL, 0);
	*target = new short[target_len];
	return MultiByteToWideChar(CP_UTF8, 0, source, source_len, (WCHAR*)*target, target_len);	
}
size_t CodeConverter::GB2312ToUtf8(char *source, size_t source_len, char **target)
{
	//转换GB2312为Unicode
    short *unicode;
	size_t unicode_len = GB2312ToUnicode(source, source_len, &unicode);

	//转换Unicode为Utf8
	size_t target_len = UnicodeToUtf8(unicode, unicode_len, target);
    delete[] unicode;        
	return target_len;
}
size_t CodeConverter::Utf8ToGB2312(char *source, size_t source_len, char **target)
{
	//转换Utf8为Unicode
    short *unicode;
	size_t unicode_len = Utf8ToUnicode(source, source_len, &unicode);

	//转换Unicode为BG2312
	size_t target_len = UnicodeToGB2312(unicode, unicode_len, target);
    delete[] unicode;        
	return target_len;
}
//从输入流中读入n个完整的utf8
int CodeConverter::GetUtf8Array(ifstream &source, char **utf8, size_t n)
{
    char c, c1, c2;
	size_t i = 0, j = 0;
	*utf8 = new char[3 * n];	
	source.read(&c, 1);
	while(!source.eof())
	{
		if((c & 0x80) == 0)   // UCS-2范围：0x0000 - 0x007F
		{
			(*utf8)[j++] = c;
			i++;
		}
		else
		{
			if((c & 0xF0) == 0xE0) // UCS-2范围：0x0800 - 0xFFFF
			{
				source.read(&c1, 1);  // 读第二个字节
				if(source.eof())
				{
					return -1; // Utf8编码不完整
				}
				else
				{
					source.read(&c2, 1); // 读第三个字节
					if(source.eof())
					{
						return -1; // Utf8编码不完整
					}
					if(((c1 & 0xC0) == 0x80) && ((c2 & 0xC0) == 0x80))
					{
						(*utf8)[j++] = c;
						(*utf8)[j++] = c1;
						(*utf8)[j++] = c2;
						i++;
					}
					else
						return -1; // Utf8编码不完整
				}
			}
			else  // UCS-2范围：0x0080 - 0x07FF
			{
				source.read(&c1, 1);  // 读第二个字节
				if(source.eof())
				{
					return -1; // Utf8编码不完整
				}
				else
				{
                    if((c1 & 0xC0) == 0x80)
					{
						(*utf8)[j++] = c;
						(*utf8)[j++] = c1;
						i++;
					}
					else
						return -1; // Utf8编码不完整
				}
			}
		}
		if(i == n) 
		{
			return j;
		}
		source.read(&c, 1);
	}
	return j;
}

size_t CodeConverter::Utf8ToGB2312(char *source, char *target)
{
    char *utf8;
    char *gb2312;
    size_t size, n = 0;
    ifstream source_stream(source);
    ofstream target_stream(target);
    while((size = GetUtf8Array(source_stream, &utf8, 300)) > 0)
    {
        size = Utf8ToGB2312(utf8, (size_t)size, &gb2312);
        for(size_t i = 0; i < size; i++)
            target_stream << gb2312[i];
        delete[] utf8;
        delete[] gb2312;
        n += size;
    }
    source_stream.close();
    target_stream.close();
    return n;
}
#endif