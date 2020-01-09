//
// Created by caesar on 2020/1/6.
//

#ifndef TOOLS_MEMORY_SHARE_H
#define TOOLS_MEMORY_SHARE_H


#ifdef _WIN32
#include <Windows.h>
#endif

struct memory_share_data_4k {
    unsigned char flag;
    long int size;
    unsigned char data[1024 * 4];
};
struct memory_share_data_8k {
    unsigned char flag;
    long int size;
    unsigned char data[1024 * 8];
};
enum memory_share_data_type {
//    len_4k,
            len_8k,
};

class memory_share {
private:
    static const char *TAG;
private:
#ifdef _WIN32
    HANDLE shared_file_handler = INVALID_HANDLE_VALUE;
    HANDLE dump_file_descriptor = INVALID_HANDLE_VALUE;
#else
#endif
    unsigned int key;
    bool is_parent = false;
    memory_share_data_type len_type = len_8k;
    memory_share_data_8k *data8k = nullptr;
public:
    explicit memory_share(unsigned int key, memory_share_data_type len_type = len_8k);

    ~memory_share();

    /**
     * init memory share
     * @return success
     */
    bool Init(bool _is_parent = false);

    /**
     * free memory share
     */
    void Free();

    int write(void *data, int len);

    int read(void *data, int len);

    int check_exist();

    /**
     * check busy flag
     * @return -1 is null; 0 not busy; 1 is busy
     */
    int is_busy();

};


#endif //TOOLS_MEMORY_SHARE_H
