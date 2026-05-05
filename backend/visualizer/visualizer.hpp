#pragma once
#include "../lowering/lowered_model.hpp"
#include <string>

namespace flux {

class Visualizer {
public:
  // 生成 DOT (Graphviz) 格式的状态机图
  std::string ToDot(const LoweredProgram &program) const;

  // 生成 Mermaid 格式的状态机图
  std::string ToMermaid(const LoweredProgram &program) const;

private:
  // 查找状态名
  std::string FindStateName(const LoweredMachine &machine, int32_t tag) const;
  // 查找事件名（排除 timeout 事件）
  std::string FindEventName(const LoweredMachine &machine, int32_t trigger_id) const;
  // 生成边标签
  std::string MakeEdgeLabel(const LoweredMachine &machine, const LoweredTransition &trans) const;
  // 获取 after 时间描述
  std::string FormatDuration(const LoweredTimerSpec &spec) const;
};

} // namespace flux
