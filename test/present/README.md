# FluxState 现实场景演示：智能路口行人过街控制

这个目录给出一个可以用于课程展示或项目演示的 FluxState 示例。场景来自真实城市交通控制：路口默认允许车辆通行，当行人按下过街按钮时，控制器进入黄灯准备、行人通行、行人闪烁提示，再回到车辆绿灯；如果急救车、消防车等紧急车辆到来，系统会立即进入紧急优先状态；维护人员也可以把路口切到维护模式。

示例文件：

- `smart_crossing.fs`：FluxState 源码。
- `run_demo.sh`：一键生成 LLVM IR、状态图、目标文件和可执行文件。

## 为什么这个场景适合 FluxState

交通路口天然是状态机：

- “车辆绿灯”“车辆黄灯”“行人通行”“紧急优先”“维护模式”都是离散状态。
- “行人按键”“紧急车辆到达”“维护开启/关闭”都是外部事件。
- “黄灯等待 1 秒后进入行人通行”“行人通行 1 秒后进入闪烁提示”是定时迁移。
- 控制器和路边显示屏之间通过事件通信，适合用 `emit` 建模。

这类逻辑如果直接写成一堆 `if` / `switch`，很容易把状态、事件和定时器混在一起。FluxState 的好处是把这些业务规则集中写成状态迁移。

## 业务模型

示例里有两个 machine。

`CrossingController` 代表路口主控制器：

- `VehicleGreen`：车辆通行。
- `VehicleYellow`：准备切换给行人。
- `PedWalk`：行人可通行。
- `PedFlash`：行人通行即将结束。
- `Emergency`：紧急车辆优先。
- `Maintenance`：维护模式。

`DisplayPanel` 代表路边显示屏：

- `Cars`：提示车辆可通行。
- `Waiting`：提示行人等待。
- `Walk`：提示行人可走。
- `Flashing`：提示尽快完成通行。
- `Alert`：提示紧急优先。
- `Service`：提示维护中。

## 关键源码片段

行人按下按钮后，控制器从车辆绿灯进入黄灯准备，并通知显示屏展示等待信息：

```flux
from VehicleGreen on PedButton(sensor_id) when emergency_active == false && maintenance_mode == false -> VehicleYellow
do {
    ped_waiting = true;
    last_sensor_id = sensor_id;
    phase = "prepare_walk";
    emit(DisplayPanel, ShowWaiting(sensor_id));
}
```

黄灯持续一段时间后，控制器进入行人通行状态，并通知显示屏：

```flux
from VehicleYellow after 1s when ped_waiting == true -> PedWalk
do {
    phase = "walk";
    emit(DisplayPanel, ShowWalk(last_sensor_id));
}
```

紧急车辆事件可以打断普通通行流程：

```flux
from PedWalk on EmergencyOn(code) -> Emergency
do {
    emergency_active = true;
    emergency_code = code;
    phase = "emergency";
    emit(DisplayPanel, ShowEmergency(code));
}
```

## 运行演示

### Docker（推荐）

从仓库根目录执行：

```bash
docker build -t fluxstate .
docker run --rm -v "${PWD}/test/present/out:/workspace/test/present/out" fluxstate ./test/present/run_demo.sh
```

### 本地

从仓库根目录执行：

```bash
bash test/present/run_demo.sh
```

脚本会生成这些文件：

```text
test/present/out/smart_crossing.ll
test/present/out/smart_crossing.dot
test/present/out/smart_crossing.mmd
test/present/out/smart_crossing.o
test/present/out/smart_crossing
```

也可以手动运行：

```bash
./build_ir.sh

./fluxstate_ir --dot test/present/smart_crossing.fs > test/present/out/smart_crossing.dot

./fluxstate_ir \
  --emit-exe test/present/out/smart_crossing \
  --inject CrossingController:PedButton:101 \
  --idle-timeout-ms 2000 \
  --max-runtime-ms 6000 \
  --debug \
  test/present/smart_crossing.fs

test/present/out/smart_crossing
```

`--inject CrossingController:PedButton:101` 表示程序启动时，模拟 101 号行人按钮被按下。之后的黄灯、行人通行、闪烁提示、恢复车辆绿灯，都是 `.fs` 中的状态迁移和 timer 驱动出来的。

`--debug` 选项会在运行时打印每条状态迁移轨迹：

```text
main
inject_initial_events
[CrossingController] VehicleGreen -> VehicleYellow
[DisplayPanel] Cars -> Waiting
[CrossingController] VehicleYellow -> PedWalk
[DisplayPanel] Waiting -> Walk
[CrossingController] PedWalk -> PedFlash
[DisplayPanel] Walk -> Flashing
[CrossingController] PedFlash -> VehicleGreen
[DisplayPanel] Flashing -> Cars
```

## 可以展示的编译器能力

这个例子覆盖了当前编译器已经实现的几条核心路径：

- 多 machine 编译。
- 事件参数绑定。
- guard 条件判断。
- action 中更新 machine 环境变量。
- 同一程序内跨 machine `emit`。
- `after` 定时迁移。
- LLVM IR 生成与验证。
- LLVM `--O0` / `--O2` 优化。
- 目标文件生成。
- runtime + generated runner 链接成可执行文件。
- DOT / Mermaid 状态图输出。

## 当前演示限制

当前 runtime 还没有接入真实 I/O。演示重点是证明 DSL 源码可以被编译、优化、链接并运行。可通过 `--debug` 选项观察运行时的状态迁移轨迹，也可通过 DOT / Mermaid 状态图进行静态分析。

当前语言也还没有支持 `log()`、`this`、字符串比较和块内局部变量，因此示例代码刻意避开了这些语法。

如果后续接入真实硬件或服务，这个示例里的 `--inject` 可以替换成外部 I/O 模块调用 `fs_enqueue_event`，例如按钮中断、摄像头检测、急救车优先信号或运维平台指令。
