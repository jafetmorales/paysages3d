#pragma once

#include "system_global.h"

#include <string>

namespace paysages {
namespace system {

class SYSTEMSHARED_EXPORT CacheFile {
  public:
    CacheFile(const string &module, const string &ext, const string &tag1, int tag2, int tag3, int tag4, int tag5,
              int tag6);

    bool isReadable();
    bool isWritable();
    string getPath();

  private:
    string filepath;
};
}
}
