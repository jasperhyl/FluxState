#include "runtime.h"

// 声明 IR 中导出的函数
extern void fs_module_init(void);
extern void fs_run_SyntaxCoverage(void);  // 运行 SyntaxCoverage 状态机
// 如果有多个状态机，可类似调用

int main(void) {
    fs_module_init();           // 初始化所有状态机（调用 fs_init_xxx）
    fs_run_SyntaxCoverage();    // 运行 SyntaxCoverage 状态机（进入事件循环）
    return 0;
}