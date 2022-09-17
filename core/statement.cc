#include "core/statement.h"

namespace Cobold {
// `AssignmentStatement` ================================================
std::string AssignmentStatement::TypeToString(const AssignmentType assgn_type) {
  switch (assgn_type) {
  case AssignmentType::EQ:
    return "=";
  case AssignmentType::MUL_EQ:
    return "*=";
  case AssignmentType::DIV_EQ:
    return "/=";
  case AssignmentType::MOD_EQ:
    return "%=";
  case AssignmentType::ADD_EQ:
    return "+=";
  case AssignmentType::SUB_EQ:
    return "-=";
  case AssignmentType::SHL_EQ:
    return "<<=";
  case AssignmentType::SHR_EQ:
    return ">>=";
  case AssignmentType::AND_EQ:
    return "&=";
  case AssignmentType::XOR_EQ:
    return "^=";
  case AssignmentType::OR_EQ:
    return "|=";
  }
}
AssignmentType
AssignmentStatement::TypeFromString(const std::string &assign_type) {
  if (assign_type == "=") {
    return AssignmentType::EQ;
  } else if (assign_type == "*=") {
    return AssignmentType::MUL_EQ;
  } else if (assign_type == "/=") {
    return AssignmentType::DIV_EQ;
  } else if (assign_type == "%=") {
    return AssignmentType::MOD_EQ;
  } else if (assign_type == "+=") {
    return AssignmentType::ADD_EQ;
  } else if (assign_type == "-=") {
    return AssignmentType::SUB_EQ;
  } else if (assign_type == "<<=") {
    return AssignmentType::SHL_EQ;
  } else if (assign_type == ">>=") {
    return AssignmentType::SHR_EQ;
  } else if (assign_type == "&=") {
    return AssignmentType::AND_EQ;
  } else if (assign_type == "^=") {
    return AssignmentType::XOR_EQ;
  } else if (assign_type == "|=") {
    return AssignmentType::OR_EQ;
  }
  assert(false);
}
// `AssignmentStatement` ================================================
} // namespace Cobold