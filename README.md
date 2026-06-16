# FluxState

FluxState 是一个面向状态机和事件驱动程序的 DSL 编译器。当前实现已经打通从 `.fs` 源码到 LLVM IR、LLVM 优化、目标文件和可执行文件的主链路。

## 当前能力

- 词法分析、语法分析、语义分析和 lowering。
- 生成 LLVM IR，并在输出前后执行 `verifyModule`。
- 通过 LLVM PassBuilder 执行 `--O0` / `--O2` 优化流水线。
- 输出 DOT / Mermaid 状态图。
- 输出原生目标文件 `.o`。
- 生成并链接可执行文件，包含运行时、自动生成的 runner、多 machine 轮询调度和初始事件注入。

## 环境要求

- Linux x86_64
- `clang++`
- `clang`
- `llvm-config`
- `rg`，用于测试脚本中的断言

如果系统里有 `clang++-15` / `llvm-config-15`，构建脚本会优先使用它们；否则使用 PATH 中的默认 `clang++` / `llvm-config`。

也可以通过环境变量指定工具：

```bash
CLANGXX_BIN=clang++ LLVM_CONFIG_BIN=llvm-config ./build_ir.sh
```

生成可执行文件时，链接器模块会查找 `clang`。如果不在 PATH 中，可以设置：

```bash
CC_BIN=/path/to/clang ./fluxstate_ir --emit-exe /tmp/program test/test.fs
```

## 构建

```bash
./build_ir.sh
```

构建成功后会生成：

```bash
./fluxstate_ir
```

## 基本用法

默认输出 LLVM IR 到 stdout：

```bash
./fluxstate_ir test/test.fs > test/test.ll
```

使用指定优化等级：

```bash
./fluxstate_ir --O0 test/test.fs > /tmp/program.O0.ll
./fluxstate_ir --O2 test/test.fs > /tmp/program.O2.ll
```

输出状态图：

```bash
./fluxstate_ir --dot test/test.fs > /tmp/program.dot
./fluxstate_ir --mermaid test/test.fs > /tmp/program.mmd
```

输出目标文件：

```bash
./fluxstate_ir --emit-obj /tmp/program.o test/test.fs
```

输出可执行文件：

```bash
./fluxstate_ir --emit-exe /tmp/program test/test.fs
/tmp/program
```

注入初始事件来驱动状态机：

```bash
./fluxstate_ir \
  --emit-exe /tmp/program \
  --inject SyntaxCoverage:Ping \
  --inject SyntaxCoverage:Configure:7,2,1.5,8.0,A,true,configured \
  --idle-timeout-ms 20 \
  --max-runtime-ms 300 \
  test/test.fs

/tmp/program
```

`--inject` 的格式是：

```text
Machine:Event[:arg1,arg2,...]
```

它表示生成的 runner 在 `fs_module_init()` 之后，先向指定 machine 的事件队列投递一个初始事件。`.fs` 文件仍然负责定义状态、事件、迁移、guard、action 和内部 `emit` 逻辑；`--inject` 只用于模拟外部世界给程序的启动输入。

## 测试

运行完整回归测试：

```bash
bash test/run_tests.sh
```

测试脚本会覆盖：

- 构建 `fluxstate_ir`
- 生成 LLVM IR
- 生成 DOT 图
- 生成 Mermaid 图
- 生成 `.o`
- 生成并运行无初始事件的可执行文件
- 生成并运行带 `--inject` 初始事件的可执行文件
- 检查一个语义错误用例会失败并输出诊断

如果已经手动构建过编译器，可以跳过构建步骤：

```bash
SKIP_BUILD=1 bash test/run_tests.sh
```

## 示例源码

主要测试样例在：

```text
test/test.fs
```

它覆盖了当前实现支持的核心语法：

- 多 machine
- 状态和 initial state
- machine 级变量
- event 参数
- `from ... on ... -> ...`
- `from ... after ... -> ...`
- `when` guard
- `do` action
- 同 machine 和跨 machine `emit`
- 基础标量类型：`int`、`short`、`float`、`double`、`char`、`bool`、`string`

## 当前限制

当前实现仍是编译器原型，以下能力还未完整支持：

- 不支持 `this`，`emit` 目标需要显式写 machine 名称。
- 不支持普通函数调用表达式。
- 不支持块内局部变量定义，变量需要定义在 machine 级别。
- IR 生成暂不支持字符串 `==` / `!=` 比较。
- 暂不支持 `set` / `map`。
- 暂不支持 `on_entry` / `on_exit` 生命周期动作。
- 暂不支持 `%` 取模和位运算。
- 当前 runner 使用轮询调度和 sleep，适合演示与测试，还不是完整 I/O 事件循环。
- 当前只面向 `x86_64-unknown-linux-gnu`。

## 编译流水线

```text
.fs
  -> Lexer / Parser
  -> Sema
  -> Lowering
  -> LLVM IRGen
  -> LLVM verify
  -> LLVM optimization
  -> LLVM verify
  -> object emission
  -> runtime + generated runner linking
  -> executable
```
