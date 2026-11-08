//
// Created by jan on 11/8/26.
//

#ifndef PROXYSHELL_MACROS_H
#define PROXYSHELL_MACROS_H

#define PRODUCT     "ProxyShell"
#define AUTHOR      "@jendahruza"
#define VERSION     "0.1.0"
#define BANNER      "%s, v%s\n(c) 2026 %s\n\n", PRODUCT, VERSION, AUTHOR

#define arr_size(array) (sizeof(array) / sizeof((array)[0]))
#define com_extra void* pExtra1, void* pExtra2  // Arguments of a command function
#define com_args nullptr, nullptr               // Default command function args value

#endif //PROXYSHELL_MACROS_H
