#ifndef _COMPILATION_MODEL_H_
#define _COMPILATION_MODEL_H_

#include "model/sql/sql_model.h"
#include <string>

class CompilationModel : public SQLModel {
public:
    explicit CompilationModel(const std::string& working_dir) {
        setField("cwd", working_dir);
    }

    std::string getTableName() const override {
        return "compilations";
    }

    std::string serialize() const override {
        return "INSERT INTO " + getTableName() + " (cwd) VALUES (" + fields_.at("cwd") + ")";
    }
};

#endif // _COMPILATION_MODEL_H_