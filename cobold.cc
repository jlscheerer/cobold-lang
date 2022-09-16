#include <iostream>

#include "absl/status/statusor.h"
#include "parser/parser.h"

int main(int argc, char **argv) {
  const std::string filename = "test/example.cb";
  absl::StatusOr<Cobold::SourceFile> source = Cobold::Parser::Parse(filename);
  std::cout << source.value().DebugString() << std::endl;
}