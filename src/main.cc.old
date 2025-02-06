#include "ast_analyzer/ast_analyzer.h"
#include <iostream>

// Print a separator line for better output formatting
void printSeparator() { std::cout << std::string(50, '-') << "\n"; }

int main(int argc, char *argv[]) {

  // Check if the correct number of arguments is provided
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <source-file>" << std::endl;
    return 1;
  }

  // Create an instance of ASTAnalyzer to analyze the source file
  ast::ASTAnalyzer analyzer;

  // Attempt to parse the provided source file
  if (!analyzer.parseFile(argv[1])) {
    std::cerr << "Failed to parse file" << std::endl;
    return 1;
  }

  // Display函数声明
  std::cout << "Functions:\n";
  printSeparator();
  for (const auto &func : analyzer.getFunctions()) {
    std::cout << "Function: " << func.name << "\n"
              << "  Return type: " << func.returnType << "\n"
              << "  Location: " << func.location.toString() << "\n"
              << "  Is definition: " << (func.isDefinition ? "yes" : "no")
              << "\n";

    if (!func.parameters.empty()) {
      std::cout << "  Parameters:\n";
      for (const auto &param : func.parameters) {
        std::cout << "    " << param.second << " " << param.first << "\n";
      }
    }
    std::cout << "\n";
  }

  // Display函数调用
  std::cout << "Function Calls:\n";
  printSeparator();
  for (const auto &call : analyzer.getCalls()) {
    std::cout << "Call: " << call.functionName << "\n"
              << "  Location: " << call.location.toString() << "\n";
    if (!call.arguments.empty()) {
      std::cout << "  Arguments: ";
      for (const auto &arg : call.arguments) {
        std::cout << arg << " ";
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  // Display变量声明
  std::cout << "Variables:\n";
  printSeparator();
  for (const auto &var : analyzer.getVariables()) {
    std::cout << "Variable: " << var.name << "\n"
              << "  Type: " << var.type << "\n"
              << "  Location: " << var.location.toString() << "\n";
    if (!var.initValue.empty()) {
      std::cout << "  Initial value: " << var.initValue << "\n";
    }
    std::cout << "\n";
  }

  // Display变量引用
  std::cout << "Variable References:\n";
  printSeparator();
  for (const auto &ref : analyzer.getReferences()) {
    std::cout << "Reference: " << ref.name << "\n"
              << "  Location: " << ref.location.toString() << "\n\n";
  }

  return 0;
}
