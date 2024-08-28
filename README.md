# dso_memory_stat

use for profile the so heap memory in the app.

Example:
1. run your app, and set libdso_mem_stat.so as LD_PRELOAD, profile so name as DSO_NAME
```shell
> DSO_NAME="libtest_dso1.so" LD_PRELOAD=./dso_mem_stat/libdso_mem_stat.so TRIGGER_SIGNAL=11 ./test/main_prog/main_prog
```

2. send signal 12 to get the memory
```shell
// send signal
> kill -12 `pidof main_prog`

// and the output
dso_mem_stat capture signal：12
dso libtest_dso1.so has heap memory：4000
```

