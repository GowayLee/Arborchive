#ifndef _CORE_VERSION_H_
#define _CORE_VERSION_H_

#include <clang/Basic/Version.h>

namespace ArborchiveVersion {

inline constexpr char codeql_version[] = "arborchive/1.0.0";
inline constexpr char frontend_version[] = CLANG_VERSION_STRING;

} // namespace ArborchiveVersion

#endif // _CORE_VERSION_H_
