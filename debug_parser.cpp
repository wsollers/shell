#include "shell/lexer.hpp"
#include "shell/parser.hpp"
#include <iostream>

using namespace shell;

static ParseResult parse_line(std::string const& s) {
  Lexer lx;
  auto lr = lx.lex(s + "\n");
  Parser p;
  return p.parse(lr.toks);
}

int main() {
  std::vector<std::string> test_cases = {
    "a &",
    "b",
    "c &",
    "a & b",
    "b ; c &",
    "a & b ; c &"
  };
  
  for (const auto& test : test_cases) {
    auto pr = parse_line(test);
    std::cout << "Testing: \"" << test << "\" -> ";
    
    if (pr.err.has_value()) {
      std::cout << "ERROR: " << pr.err->msg << std::endl;
    } else {
      std::cout << "SUCCESS: " << pr.seq.items.size() << " items";
      for (size_t i = 0; i < pr.seq.items.size(); ++i) {
        std::cout << " [" << i << ": bg=" << pr.seq.items[i].background << "]";
      }
      std::cout << std::endl;
    }
  }
  
  return 0;
}