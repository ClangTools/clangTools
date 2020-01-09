
#ifndef _GET_CONFIG_H_
#define _GET_CONFIG_H_

#include <string>
#include <list>

using namespace std;

class ConfigTool {
public:
    ConfigTool(const char *path);

    /**
     * 获取
     * @param section 域
     * @param key 键
     * @param defValue 默认值
     * @return
     */
    string Get(string section, string key, string defValue = "");

    /**
     * 设置
     * @param section 域
     * @param key 键
     * @param value 值
     * @return
     */
    bool Set(string section, string key, string value);

private:
    /**
     * 文件路径
     */
    string ConfPath = "";
    /**
     * 当前数据
     */
    list<string> Data;

    /**
     * 保存
     */
    void save();
};

#endif