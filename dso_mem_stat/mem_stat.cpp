#define _GNU_SOURCE

#include <mutex>
#include <map>
#include <atomic>

#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#include <cstring>
#include <signal.h>

#define STACK_INFO_LEN  1024
#define MAX_STACK_FRAMES 12

static void *(*fn_malloc)(size_t size);
static void  (*fn_free)(void *ptr);

static std::mutex pos_mtx;
static std::map<std::string, uint64_t> mem_size_map{};
static std::map<uint64_t, size_t> mem_pos_map{};

static std::mutex init_mtx;

static char dso_name[STACK_INFO_LEN];
static int filter_signal_num = 12;

static std::atomic_int check_state{0}; // 0: no check, 1:checking, 2:checked
static std::atomic_bool is_stat {false};
thread_local bool is_collect_info = false;

static void init()
{
    fn_malloc = (void*(*)(size_t))dlsym(RTLD_NEXT, "malloc");
    fn_free   = (void(*)(void*))dlsym(RTLD_NEXT, "free");

    if (!fn_malloc || !fn_free) {
        fprintf(stderr, "Error in dlsym");
        exit(1);
    }
}

void collect_info(void* ptr, size_t size)
{
    void *p_stack[MAX_STACK_FRAMES];
    char stack_info[STACK_INFO_LEN * MAX_STACK_FRAMES];

    char** p_stack_list = nullptr;
    int frames = backtrace(p_stack, MAX_STACK_FRAMES);
    p_stack_list = backtrace_symbols(p_stack, frames);
    if (p_stack_list == nullptr) {
        return;
    }

    for (int i = 0; i < frames; ++i) {
        if (p_stack_list[i] == nullptr) {
            break;
        }

        strncat(stack_info, p_stack_list[i], STACK_INFO_LEN);
        strcat(stack_info, "\n");

        if (strstr(p_stack_list[i], dso_name) != nullptr) {
            std::lock_guard<std::mutex> lock(pos_mtx);
            mem_size_map[dso_name] += size;
            mem_pos_map[uint64_t(ptr)] = size;
            fprintf(stderr, "dso_name %s in %p has size: %ld \n", dso_name, ptr, size);
        }
    }
    strcat(stack_info, "\n");
//    fprintf(stderr, "%s", stack_info); // 输出到控制台，也可以打印到日志文件中
}

void stat_signal_handler(int num)
{
    fprintf(stderr, "dso_mem_stat capture signal：%d\n",num);
    fprintf(stderr, "dso %s has heap memory：%ld\n", dso_name, mem_size_map[dso_name]);
}

void check_filter()
{
    const char* name = std::getenv("DSO_NAME");
    if (name != nullptr) {
        strncpy(dso_name, name, STACK_INFO_LEN);
        fprintf(stderr, "dso name: %s\n", name);
        fprintf(stderr, "trigger signal: %d\n", filter_signal_num);

        is_stat = true;
        signal(filter_signal_num, stat_signal_handler);
    }
}

void *malloc(size_t size)
{
    {
        std::lock_guard<std::mutex> lock(init_mtx);
        if (!fn_malloc) {
            init();
        }
    }

    int is_check = 0;
    if (check_state.compare_exchange_strong(is_check, 1)) {
        check_filter();
    }

    void *ptr = fn_malloc(size);
    fprintf(stderr, "allocated bytes memory %ld in %p\n", size, ptr);

    if (is_stat) {
        if (!is_collect_info) {
            is_collect_info = true;
            collect_info(ptr, size);
            is_collect_info = false;
        }
    }

    return ptr;
}

void free(void *ptr)
{
    fn_free(ptr);
    fprintf(stderr, "deallocated bytes memory in %p\n", ptr);

    if (is_stat && !is_collect_info && ptr != nullptr) {
        std::lock_guard<std::mutex> lock(pos_mtx);
        if (mem_pos_map.find((uint64_t)ptr) != mem_pos_map.end()) {
            mem_size_map[dso_name] -= mem_pos_map[(uint64_t)ptr];
            fprintf(stderr, "dso_name %s in %p free size: %ld \n", dso_name, ptr, mem_pos_map[(uint64_t)ptr]);
        }
    }
}