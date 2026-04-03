#pragma once
namespace flux {
enum class ValueType {
  Invalid, // 语义错误占位
  Bool,
  Char,
  Short,
  Int,
  Float,
  Double,
  String,
};

// 一元运算符
enum class UnaryOp {
  Neg,  //-
  Plus, //+
  // Not,       //!
  AddressOf, //&
  Deref,     //*
  // BitNot,    //~不支持
};

// 二元运算（不支持位运算）
enum class BinaryOp {
  // 逻辑
  LogicAnd, // &&
  LogicOr,  // ||
  // 算术
  Add, // +
  Sub, // -
  Mul, // *
  Div, // /
  // Mod, // %暂时不支持
  // 比较
  Eq, // ==
  Ne, // !=
  Lt, // <
  Le, // <=
  Gt, // >
  Ge, // >=
  // 赋值
  Assign // =
};

} // namespace flux
  //   KwMachine,
  //   KwState,
  //   KwInitial,
  //   KwEvent,
  //   KwTransition,
  //   KwGuard,
  //   KwTimeout,
  //   KwFrom,
  //   KwOn,
  //   KwWhen,
  //   KwAfter,
  //   KwDo,
  //   KwTrue,
  //   KwFalse,
  //   KwH,
  //   KwM,
  //   KwS, // time suffix: 10s -> IntegerLiteral(10), KwS