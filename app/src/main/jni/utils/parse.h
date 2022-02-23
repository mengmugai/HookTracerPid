//
// Created by Administrator on 2020-09-14.
//

#ifndef INC_01_PARSE_H
#define INC_01_PARSE_H


#include <jni.h>
#include <string>
#include <regex>


using namespace std;

class parse {

public:
    static  jstring char2jstring(JNIEnv* env,const char* pat);
    static string jstring2str(JNIEnv* env, jstring jstr);

    static vector<string> Split(const string &str, const string &pattern){
        vector<string> retList ;
        if (pattern.empty())
            return retList;
        size_t start = 0, index = str.find_first_of(pattern, 0);
        while (index != std::string::npos) {
            if (start != index)
                retList.push_back(str.substr(start, index - start));
            start = index + 1;
            index = str.find_first_of(pattern, start);
        }
        if (!str.substr(start).empty())
            retList.push_back(str.substr(start));
        return retList;
    }

};


#endif //INC_01_PARSE_H
