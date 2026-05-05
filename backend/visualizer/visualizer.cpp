#include "visualizer.hpp"
#include <sstream>

namespace flux {

std::string Visualizer::FindStateName(const LoweredMachine &machine, int32_t tag) const {
  for (const auto &state : machine.states) {
    if (state.tag == tag) {
      return state.name;
    }
  }
  return "UNKNOWN";
}

std::string Visualizer::FindEventName(const LoweredMachine &machine, int32_t trigger_id) const {
  for (const auto &event : machine.events) {
    if (event.tag == trigger_id && !event.is_timeout) {
      return event.name;
    }
  }
  return "";
}

std::string Visualizer::FormatDuration(const LoweredTimerSpec &spec) const {
  std::string result = std::to_string(spec.delay);
  switch (spec.unit) {
  case DurationLiteral::Unit::MilliSeconds:
    result += "ms";
    break;
  case DurationLiteral::Unit::Seconds:
    result += "s";
    break;
  case DurationLiteral::Unit::Minutes:
    result += "m";
    break;
  }
  return result;
}

std::string Visualizer::MakeEdgeLabel(const LoweredMachine &machine,
                                       const LoweredTransition &trans) const {
  std::string label;

  if (trans.trigger_kind == LoweredTriggerKind::OnEvent) {
    std::string event_name = FindEventName(machine, trans.trigger_id);
    // 查找原始 transition 的 event bindings
    std::string params;
    if (trans.source) {
      if (const auto *trigger = std::get_if<OnEventTrigger>(&trans.source->trigger)) {
        if (!trigger->bindings.empty()) {
          params = "(";
          for (size_t i = 0; i < trigger->bindings.size(); ++i) {
            if (i > 0)
              params += ", ";
            params += trigger->bindings[i];
          }
          params += ")";
        }
      }
    }
    label = "on " + event_name + params;
  } else {
    if (trans.timer.has_value()) {
      label = "after " + FormatDuration(trans.timer.value());
    } else {
      label = "after ?";
    }
  }

  if (trans.source && trans.source->guard) {
    label += " [when ...]";
  }

  return label;
}

// ===== DOT 格式 =====
std::string Visualizer::ToDot(const LoweredProgram &program) const {
  std::ostringstream oss;

  for (const auto &machine : program.machines) {
    oss << "digraph " << machine.name << " {\n";
    oss << "    rankdir=TB;\n";
    oss << "    node [shape=circle, style=filled, fillcolor=white, fontname=\"Arial\"];\n";
    oss << "    edge [fontname=\"Arial\", fontsize=10];\n\n";

    // 初始状态标记（用一个不可见节点 + 箭头）
    oss << "    __initial [shape=point, width=0, style=invis];\n";
    std::string init_name = FindStateName(machine, machine.initial_state_tag);
    oss << "    __initial -> " << init_name << " [label=\"\"];\n\n";

    // 声明所有状态节点，初始状态用特殊颜色
    for (const auto &state : machine.states) {
      if (state.tag == machine.initial_state_tag) {
        oss << "    " << state.name << " [fillcolor=\"#4CAF50\", fontcolor=white];\n";
      } else {
        oss << "    " << state.name << ";\n";
      }
    }
    oss << "\n";

    // 生成转换边
    for (const auto &trans : machine.transitions) {
      std::string src = FindStateName(machine, trans.src_state_tag);
      std::string dst = FindStateName(machine, trans.dst_state_tag);
      std::string label = MakeEdgeLabel(machine, trans);

      oss << "    " << src << " -> " << dst << " [label=\"" << label << "\"];\n";
    }

    oss << "}\n\n";
  }

  return oss.str();
}

// ===== Mermaid 格式 =====
std::string Visualizer::ToMermaid(const LoweredProgram &program) const {
  std::ostringstream oss;

  for (size_t mi = 0; mi < program.machines.size(); ++mi) {
    const auto &machine = program.machines[mi];
    oss << "stateDiagram-v2\n";
    oss << "    direction TB\n\n";

    // 初始状态
    std::string init_name = FindStateName(machine, machine.initial_state_tag);
    oss << "    [*] --> " << init_name << "\n\n";

    // 生成转换
    for (const auto &trans : machine.transitions) {
      std::string src = FindStateName(machine, trans.src_state_tag);
      std::string dst = FindStateName(machine, trans.dst_state_tag);
      std::string label = MakeEdgeLabel(machine, trans);

      oss << "    " << src << " --> " << dst << " : " << label << "\n";
    }

    // 如果有多个 machine，加空行分隔
    if (mi < program.machines.size() - 1) {
      oss << "\n";
    }
  }

  return oss.str();
}

} // namespace flux
