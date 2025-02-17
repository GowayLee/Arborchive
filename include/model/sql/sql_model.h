#ifndef _SQL_MODEL_H_
#define _SQL_MODEL_H_

#include <map>
#include <string>

class SQLModel {
public:
  virtual ~SQLModel() = default;
  virtual std::string getTableName() const = 0;
  virtual std::string serialize() const = 0;
};

#endif // _SQL_MODEL_H_