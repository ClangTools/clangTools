//
// Created by caesar kekxv on 2021/4/11.
//

#ifndef TERMINALSERVICE_LOGGER_ATTR_H
#define TERMINALSERVICE_LOGGER_ATTR_H

#ifndef LOGGER_ATTR
#ifdef __GNUC__
#define LOGGER_ATTR(x, y) __attribute__((format(printf, x, y)))
#else
#define LOGGER_ATTR(x, y)
#endif
#endif // LOGGER_ATTR
#ifndef _Printf_format_string_
#define _Printf_format_string_
#endif

#ifdef WIN32
#ifdef _Tools_HEADER_
#define DLL_logger_Export  __declspec(dllexport)
#else
#define DLL_logger_Export  __declspec(dllimport)
#endif
#else
#define DLL_logger_Export
#endif

#endif //TERMINALSERVICE_LOGGER_ATTR_H
