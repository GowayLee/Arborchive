#ifndef _COMPILATION_ARGS_MODEL_H_
#define _COMPILATION_ARGS_MODEL_H_

#include "model/sql/sql_model.h"

class CompilationArgsModel : public SQLModel {
public:
  CompilationArgsModel(int compilation_id, int num) {
    setField("id", compilation_id);
    setField("num", num);
  }

  std::string getTableName() const override { return "compilation_args"; }

  void setArg(const std::string &arg) { setField("arg", arg); }
  std::string serialize() const override {
    std::string escaped_arg = fields_.at("arg");
    // 转义单引号
    size_t pos = 0;
    while ((pos = escaped_arg.find("'", pos)) != std::string::npos) {
      escaped_arg.replace(pos, 1, "''");
      pos += 2;
    }
    return "INSERT INTO " + getTableName() + " (id, num, arg) VALUES (" +
           fields_.at("id") + ", " + fields_.at("num") + ", " + "'" +
           escaped_arg + "')";
  }
};

#endif // _COMPILATION_ARGS_MODEL_H_