/*
 * The MIT License
 *
 * Copyright 2023 Andrea Vouk.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*------------------------------------------------------------------------------
    anv_trace
--------------------------------------------------------------------------------

  very simple and not thread-safe logging lib.

  simple example:

    #define ANV_TRACE_IMPLEMENTATION
    #include <anv_trace.h>

    int main(void)
    {
        anv_trace_init(stdout);

        anv_trace_enter();
        anv_traced("Hello %s!", "Debug");
        anv_tracei("Hello %s!", "Info");
        anv_tracew("Hello %s!", "Warning");
        anv_tracee("Hello %s!", "Error");
        anv_tracef("Hello %s!", "Fatal");
        anv_trace_leave();

        anv_trace_quit();
    }

------------------------------------------------------------------------------*/

#ifndef ANV_TRACE_H
#define ANV_TRACE_H

#include <stdio.h>

#ifndef ANV_TRACE_EXP
    #define ANV_TRACE_EXP extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** verbose. */
#define ANV_TRACE_DEBUG   0
/** common log message. */
#define ANV_TRACE_INFO    1
/** warning log message. */
#define ANV_TRACE_WARNING 2
/** current operation aborted. the program will not crash. */
#define ANV_TRACE_ERROR   3
/** critical failure. program crash imminent. */
#define ANV_TRACE_FATAL   4

ANV_TRACE_EXP void anv_trace_init(FILE *file);
ANV_TRACE_EXP void anv_trace_quit(void);

ANV_TRACE_EXP void anv_trace_(
    const char *filename,
    int line,
    const char *func,
    int ctr,
    const char *format,
    ...
);

#define anv_trace(ctr, format, ...)                                            \
    anv_trace_(__FILE__, __LINE__, __FUNCTION__, ctr, format, __VA_ARGS__)

#define anv_traced(format, ...) anv_trace(ANV_TRACE_DEBUG, format, __VA_ARGS__)

#define anv_tracei(format, ...) anv_trace(ANV_TRACE_INFO, format, __VA_ARGS__)

#define anv_tracew(format, ...)                                                \
    anv_trace(ANV_TRACE_WARNING, format, __VA_ARGS__)

#define anv_tracee(format, ...) anv_trace(ANV_TRACE_ERROR, format, __VA_ARGS__)

#define anv_tracef(format, ...) anv_trace(ANV_TRACE_FATAL, format, __VA_ARGS__)

#define anv_trace_enter() anv_traced("<< entering \"%s\"", __FUNCTION__)
#define anv_trace_leave() anv_traced(">> leaving  \"%s\"", __FUNCTION__)

#ifdef __cplusplus
}
#endif

#ifdef ANV_TRACE_IMPLEMENTATION

    #include <stdarg.h>
    #include <string.h>
    #include <time.h>

static FILE *anv_trc__g_out = NULL;

    #ifndef anv_trc__assert
        #include <assert.h>
        #define anv_trc__assert(x) assert(x)
    #endif

    #define ANV_TRACE__MAX_LOGMESSAGE_LEN 256

    #ifdef ANV_TRACE_NO_PRETTY_PRINT
        #define ANV_TRACE__FORMAT_HEADER                                       \
            "== [MESSAGE] [%s: LINE | %s] Begin Trace (%d/%.2d/%.2d - "        \
            "%.2d:%.2d:%.2d)\n\n"
        #define ANV_TRACE__FORMAT_MESSAGE "-- [%s] [%s:%d | %s] %s\n"
        #define ANV_TRACE__FORMAT_FOOTER                                       \
            "\n== [MESSAGE] [%s: LINE | %s] End Trace   (%d/%.2d/%.2d - "      \
            "%.2d:%.2d:%.2d)\n"
    #else
        #define ANV_TRACE__FORMAT_HEADER                                       \
            "== [MESSAGE] [%25s: LINE | %-20s] Begin Trace (%d/%.2d/%.2d - "   \
            "%.2d:%.2d:%.2d)\n\n"
        #define ANV_TRACE__FORMAT_MESSAGE "-- [%-7s] [%25s:%5d | %-20s] %s\n"
        #define ANV_TRACE__FORMAT_FOOTER                                       \
            "\n== [MESSAGE] [%25s: LINE | %-20s] End Trace   (%d/%.2d/%.2d - " \
            "%.2d:%.2d:%.2d)\n"
    #endif

static const char *
anv_trc__get_filename(const char *file)
{
    #if defined(_WIN32) || defined(__CYGWIN__)
    return (strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file);
    #else
    return (strrchr(file, '/') ? strrchr(file, '/') + 1 : file);
    #endif
}

static const char *
anv_trc__tostr(int ctr)
{
    switch (ctr) {
        case ANV_TRACE_DEBUG:
            return "Debug";
        case ANV_TRACE_INFO:
            return "Info";
        case ANV_TRACE_WARNING:
            return "Warning";
        case ANV_TRACE_ERROR:
            return "Error";
        case ANV_TRACE_FATAL:
            return "Fatal";
        default:
            anv_trc__assert(0);
            return "Unknown";
    }
}

static void
anv_trc__print_sep(const char *format)
{
    time_t raw = time(NULL);
    struct tm *t = localtime(&raw);
    fprintf(
        anv_trc__g_out,
        format,
        "FILENAME",
        "FUNCTION",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec
    );
}

void
anv_trace_init(FILE *file)
{
    anv_trc__assert(file);
    anv_trc__g_out = file;
    anv_trc__print_sep(ANV_TRACE__FORMAT_HEADER);
}

void
anv_trace_quit(void)
{
    anv_trc__assert(anv_trc__g_out);
    anv_trc__print_sep(ANV_TRACE__FORMAT_FOOTER);
    anv_trc__g_out = NULL;
}

void
anv_trace_(
    const char *filename,
    int line,
    const char *func,
    int ctr,
    const char *format,
    ...
)
{
    anv_trc__assert(anv_trc__g_out);

    char buff[ANV_TRACE__MAX_LOGMESSAGE_LEN];
    const char *trstr = anv_trc__tostr(ctr);

    sprintf(
        buff,
        ANV_TRACE__FORMAT_MESSAGE,
        trstr,
        anv_trc__get_filename(filename),
        line,
        func,
        format
    );

    va_list vl;
    va_start(vl, format);
    vfprintf(anv_trc__g_out, buff, vl);
    va_end(vl);
}

#endif

#endif /* ANV_TRACE_H */
