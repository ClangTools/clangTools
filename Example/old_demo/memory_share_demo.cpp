//
// Created by caesar on 2020/1/6.
//

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#else

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#endif
#include <memory_share.h>

using namespace std;

#ifdef false
void writeMemory()
{
    // define shared data
    char *shared_file_name = "my_shared_memory";
    unsigned long buff_size = 4096;
    char share_buffer[] = "greetings, hello world";
    //MyData share_buffer("Tom", 18);

    // create shared memory file
    dump_file_descriptor = CreateFile(shared_file_name,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS, // open exist or create new, overwrite file
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (dump_file_descriptor == INVALID_HANDLE_VALUE)
        cout << "create file error" << endl;

    HANDLE shared_file_handler = CreateFileMapping(
        dump_file_descriptor, // Use paging file - shared memory
        NULL,                 // Default security attributes
        PAGE_READWRITE,       // Allow read and write access
        0,                    // High-order DWORD of file mapping max size
        buff_size,            // Low-order DWORD of file mapping max size
        shared_file_name);    // Name of the file mapping object

    if (shared_file_handler)
    {
        // map memory file view, get pointer to the shared memory
        LPVOID lp_base = MapViewOfFile(
            shared_file_handler,  // Handle of the map object
            FILE_MAP_ALL_ACCESS,  // Read and write access
            0,                    // High-order DWORD of the file offset
            0,                    // Low-order DWORD of the file offset
            buff_size);           // The number of bytes to map to view



        // copy data to shared memory
        memcpy(lp_base, &share_buffer, sizeof(share_buffer));

        FlushViewOfFile(lp_base, buff_size); // can choose save to file or not

        // process wait here for other task to read data
        cout << "already write to shared memory, wait ..." << endl;
        //cout << share_buffer << endl;
        this_thread::sleep_for(chrono::seconds(10));

        // close shared memory file
        UnmapViewOfFile(lp_base);
        CloseHandle(shared_file_handler);
        CloseHandle(dump_file_descriptor);
        //unlink(shared_file_name);
        cout << "shared memory closed" << endl;
    }
    else
        cout << "create mapping file error" << endl;
}
void readMemory()
{
    char *shared_file_name = "my_shared_memory";

    // open shared memory file
    HANDLE shared_file_handler = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        NULL,
        shared_file_name);

    if (shared_file_handler)
    {
        LPVOID lp_base = MapViewOfFile(
            shared_file_handler,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            0);

        // copy shared data from memory
        cout << "read shared data: " << endl;
        const unsigned long buff_size = 4096;
        //char share_buffer[buff_size] = { 0 };
        //strcpy(share_buffer, (char *)lp_base);
        char *share_buffer = (char *)lp_base;

        cout << share_buffer << endl;

        /*MyData *my_data = (MyData *)lp_base;
        cout << my_data->name << " " << my_data->age << endl;*/

        // close share memory file
        UnmapViewOfFile(lp_base);
        CloseHandle(shared_file_handler);
    }
    else
        cout << "open mapping file error" << endl;
}
#else

#endif

int main() {
    memory_share memoryShare(0x366378);
    memory_share memoryShare1(0x366378);

    memoryShare.Init(true);
    memoryShare1.Init();

    printf("\n\n============================>\n");
    char str[] = "12345678901234567890";
    printf("\twrite : %s\n", str);
    if (memoryShare.check_exist())
        memoryShare.write((void *) str, sizeof(str));
    memset(str, 0x00, sizeof(str));
    if (memoryShare1.check_exist())
        memoryShare1.read((void *) str, sizeof(str));
    printf("\tread  : %s\n", str);
    printf("============================>\n");

    memoryShare1.Free();
    memoryShare.Free();
    printf("\n\n============================>\n\n");
    system("ipcs -m");
    printf("============================>\n");
    return EXIT_SUCCESS;
}