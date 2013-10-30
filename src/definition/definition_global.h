#ifndef DEFINITION_GLOBAL_H
#define DEFINITION_GLOBAL_H

/* Shared object helpers */
#ifdef __cplusplus
#  include <QtCore/qglobal.h>
#else
#  if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#    define Q_DECL_EXPORT __declspec(dllexport)
#    define Q_DECL_IMPORT __declspec(dllimport)
#  else
#    define Q_DECL_EXPORT
#    define Q_DECL_IMPORT
#  endif
#endif
#if defined(DEFINITION_LIBRARY)
#  define DEFINITIONSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DEFINITIONSHARED_EXPORT Q_DECL_IMPORT
#endif

/* Namespace using */
#ifdef __cplusplus
namespace paysages
{
    namespace system {}
    namespace basics {}
    namespace definition {}
}
using namespace paysages::system;
using namespace paysages::basics;
using namespace paysages::definition;
#endif

/* Global imports */

#endif // DEFINITION_GLOBAL_H
