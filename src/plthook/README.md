# PLTHook

Fork : https://github.com/kubo/plthook

```c++
#include <dlfcn.h>
#include <cstdio>
#include <logger.h>
#include <plthook.h>

int print_success() {
    return 0;
}

int install_hook_function(const char *so_path) {
    plthook_t *plthook;

    if (plthook_open(&plthook, so_path) != 0) {
        printf("plthook_open error: %s\n", plthook_error());
        return -1;
    }
    if (plthook_replace(plthook, "print_fail", (void *) print_success, nullptr) != 0) {
        printf("plthook_replace error: %s\n", plthook_error());
        plthook_close(plthook);
        return -1;
    }
    plthook_close(plthook);
    return 0;
}

int main(int argc, char *argv[]) {
    logger::instance()->init_default();

    void *handle = dlopen("./libprint_fail.so", RTLD_LAZY);
    if (!handle) {
        logger::instance()->e(__FILENAME__, __LINE__, "dlopen get error: %s", dlerror());
        return -1;
    }

    install_hook_function("./libprint_fail.so");

    int (*func)();
    func = (int (*)()) dlsym(handle, "print_fail");
    if (func == nullptr) {
        logger::instance()->e(__FILENAME__, __LINE__, "获取函数失败 get error: %s", dlerror());
        return -1;
    }

    int ret = (*func)();
    logger::instance()->i(__FILENAME__, __LINE__, "print_fail : %d", ret);
    return 0;
}
```