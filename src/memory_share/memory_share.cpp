//
// Created by caesar on 2020/1/6.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <logger.h>
#include <memory_share.h>

using namespace std;

#ifdef _WIN32
#else

#include <sys/ipc.h>
#include <sys/shm.h>

#endif


#ifndef memory_share_data_flag
#define memory_share_data_flag
#define memory_share_data_flag_read     0b0000001
#define memory_share_data_flag_write    0b0000010
#define memory_share_data_flag_ready_1  0b0000100
#define memory_share_data_flag_ready_2  0b0001000
#define memory_share_data_flag_busy     0b1000000
#endif

#ifdef __FILENAME__
const char *memory_share::TAG = __FILENAME__;
#else
const char* memory_share::TAG = "memory_share";
#endif

memory_share::memory_share(unsigned int key, memory_share_data_type len_type) {
    this->key = key;
    this->len_type = len_type;
}

memory_share::~memory_share() {
    Free();
}

bool memory_share::Init(bool _is_parent) {
    this->is_parent = _is_parent;
#ifdef _WIN32
    string shared_file_name = "shared_memory" + to_string(key);
    int buff_size = sizeof(memory_share_data_8k);
    // create shared memory file
    HANDLE dump_file_descriptor = CreateFile(shared_file_name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS, // open exist or create new, overwrite file
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (dump_file_descriptor == INVALID_HANDLE_VALUE) {
        logger::instance()->e(TAG, __LINE__, "shmget error\n");
        return false;
    }

    shared_file_handler = CreateFileMapping(
        dump_file_descriptor, // Use paging file - shared memory
        NULL,                 // Default security attributes
        PAGE_READWRITE,       // Allow read and write access
        0,                    // High-order DWORD of file mapping max size
        buff_size,            // Low-order DWORD of file mapping max size
        shared_file_name.c_str());    // Name of the file mapping object

    if (shared_file_handler == INVALID_HANDLE_VALUE) {
        logger::instance()->e(TAG, __LINE__, "shared_file_handler error\n");
        CloseHandle(dump_file_descriptor);
        dump_file_descriptor = INVALID_HANDLE_VALUE;
        return false;
    }
    if (shared_file_handler)
    {
        // map memory file view, get pointer to the shared memory
         data8k = (memory_share_data_8k *)  MapViewOfFile(
            shared_file_handler,  // Handle of the map object
            FILE_MAP_ALL_ACCESS,  // Read and write access
            0,                    // High-order DWORD of the file offset
            0,                    // Low-order DWORD of the file offset
            buff_size);           // The number of bytes to map to view
        if (data8k == (void *) -1)data8k = nullptr;
    }
#else
    int shm_id;
    //  创建共享内存
    shm_id = shmget(key, sizeof(memory_share_data_8k),
                    _is_parent ? 0640 | IPC_CREAT | IPC_EXCL : 0640);
    if (shm_id == -1) {
        logger::instance()->e(TAG, __LINE__, "shmget error\n");
        return false;
    }
    //  将这块共享内存区附加到自己的内存段
    if (len_type == len_8k) { data8k = (memory_share_data_8k *) shmat(shm_id, nullptr, 0); }

    if (data8k == (void *) -1)data8k = nullptr;
#endif
    return data8k != nullptr;
}

void memory_share::Free() {
#ifdef _WIN32
    // close shared memory file
    UnmapViewOfFile(data8k);
    data8k = nullptr;

    CloseHandle(shared_file_handler);
    CloseHandle(dump_file_descriptor);

    shared_file_handler = INVALID_HANDLE_VALUE;
    dump_file_descriptor = INVALID_HANDLE_VALUE;
#else
    int shm_id;
    //  将这块共享内存区附加到自己的内存段
    if (len_type == len_8k) {
        if (data8k != nullptr) {
            shmdt(data8k);
            data8k = nullptr;
        }
        //  首先检查共享内存是否存在，存在则先删除
        shm_id = shmget(key, sizeof(memory_share_data_8k), 0640);
        if (shm_id != -1) {
            data8k = (memory_share_data_8k *) shmat(shm_id, nullptr, 0);
            if (data8k == (void *) -1)data8k = nullptr;
            if (data8k != nullptr) {
                shmdt(data8k);
                data8k = nullptr;
                if (is_parent) {
                    //  删除共享内存
                    if (shmctl(shm_id, IPC_RMID, nullptr) == -1) {
                        logger::instance()->e(TAG, __LINE__, " delete error %d:%s", errno,strerror(errno));
                    } else {
                        shm_id = -1;
                    }
                }
            }
        }
    }
#endif
}


int memory_share::write(void *data, int len) {
    //  将这块共享内存区附加到自己的内存段
    if (len_type == memory_share_data_type::len_8k)
	{
        if (len >= 1024 * 8)return -2;
        if (data8k == nullptr)return -1;
        if (data8k->flag & (unsigned char) memory_share_data_flag_busy) { return 0; }
        data8k->flag = (unsigned char) memory_share_data_flag_busy | (unsigned char) memory_share_data_flag_write;
        data8k->size = len;
        memcpy(data8k->data, data, len);
        data8k->flag = ((unsigned char) (is_parent ? memory_share_data_flag_ready_1 : memory_share_data_flag_ready_2)) | (unsigned char) memory_share_data_flag_write;
    }
    return 0;
}

int memory_share::read(void *data, int len) {
    //  将这块共享内存区附加到自己的内存段
    if (len_type == memory_share_data_type::len_8k)
	{
        if (data8k == nullptr)return -1;
        if (data8k->flag & (unsigned char) memory_share_data_flag_busy) { return 0; }
        if (!(data8k->flag &
              (unsigned char) (is_parent ? memory_share_data_flag_ready_2
                                         : memory_share_data_flag_ready_1))) { return 0; }
        data8k->flag = (unsigned char) memory_share_data_flag_busy | (unsigned char) memory_share_data_flag_read;
        if (len < data8k->size)
			return -2;
        memcpy(data, data8k->data, data8k->size);
        data8k->flag = (unsigned char) (is_parent ? memory_share_data_flag_ready_1 : memory_share_data_flag_ready_2) |
                       (unsigned char) memory_share_data_flag_read;
    }
    return 0;
}

int memory_share::check_exist() {
#ifdef _WIN32
    return 1;
#else
    int shm_id;
    //  创建共享内存
    shm_id = shmget(key, len_type == len_8k ? sizeof(memory_share_data_8k) : sizeof(memory_share_data_4k),
                    0640);
    return shm_id != -1;
#endif
}

int memory_share::is_busy() {
    //  将这块共享内存区附加到自己的内存段
    if (len_type == memory_share_data_type::len_8k)
	{
        if (data8k == nullptr)return -1;
        if (data8k->flag & (unsigned char) memory_share_data_flag_busy) { return 1; }
    }
    return 0;
}
