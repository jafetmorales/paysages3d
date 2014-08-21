#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "system_global.h"

namespace paysages {
namespace system {

class SYSTEMSHARED_EXPORT FileSystem
{
public:
    /**
     * Get an absolute path to a temporary file.
     *
     * filename must not contain directory separators.
     */
    static std::string getTempFile(const std::string &filename);
};

}
}

#endif // FILESYSTEM_H
