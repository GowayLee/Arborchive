#!/bin/bash

../codeql-db-extractor/codeql-to-souffle.py --sqlite-db ./tests/ast.db --language cpp --output local.dl --prefix local
rm tests/datalog.local/1/local.dl
mv ./local.dl ./tests/datalog.local/1/
