# judge-runner
judge-runner
## 获取可执行程序的时间、空间消耗
核心流程：
* vfork创建子进程
* ptrace关联父子进程
* wait4获取子进程资源情况
* 主进程轮训子进程
