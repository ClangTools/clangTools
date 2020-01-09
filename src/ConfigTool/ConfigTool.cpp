#include <utility>

#include "ConfigTool.h"
#include <map>

#include<fstream>
#include<iostream>


using namespace std;


std::string &trim(std::string &, char c = ' ');

/**
 * @brief 清除尾部空格
 *
 * @param s
 * @return std::string&
 */
std::string &trim(std::string &s, char c) {
    if (s.empty()) {
        return s;
    }

    s.erase(0, s.find_first_not_of(c));
    s.erase(s.find_last_not_of(c) + 1);
    return s;
}

ConfigTool::ConfigTool(const char *path) {
    ConfPath = path;
    Data.clear();


    fstream confFile(ConfPath);
    if (confFile.is_open()) {
        string temp;
        bool SectionFlag = false;
        while (getline(confFile, temp)) {
            Data.push_back(temp);
        }
        confFile.close();
    }
};


string ConfigTool::Get(string section, string key, string defvalue) {
    bool SectionFlag = false;
    for (list<string>::iterator it = Data.begin(); it != Data.end(); it++) {
        string item = *it;
        if (!SectionFlag && item.find("[" + section + "]") != string::npos) {
            SectionFlag = true;
            continue;
        }
        if (SectionFlag && item[0] == '[' && item.find(']') != string::npos) {
            break;
        }
        if (SectionFlag && item.find(key) != string::npos && item.find('=') != string::npos) {
            string val = item.substr(item.find('=') + 1);
            return trim(val);
        }
    }
    return defvalue;
}

bool ConfigTool::Set(string section, string key, string value) {
    bool SectionFlag = false;

    list<string>::iterator iter;
    for (iter = Data.begin(); iter != Data.end(); iter++) {

        if (!SectionFlag && iter->find("[" + section + "]") != string::npos) {
            SectionFlag = true;
            continue;
        }
        if (SectionFlag && (*iter)[0] == '[' && iter->find(']') != string::npos) {
            Data.insert(iter, 1, key.append("=").append(value));
            save();
            return true;
        }
        if (SectionFlag && iter->find(key) != string::npos && iter->find('=') != string::npos) {
            *iter = key.append("=").append(value);
            save();
            return true;
        }

    }
    Data.push_back(("[" + section).append("]"));
    Data.push_back(key.append("=").append(value));
    save();
    return true;
}

void ConfigTool::save() {
    ofstream confFile(ConfPath);
    if (confFile.is_open()) {
        list<string>::iterator iter;
        for (iter = Data.begin(); iter != Data.end(); iter++) {
            confFile << *iter << endl;
            confFile.seekp(0, ios::end);
        }
        confFile.close();
    }
}
