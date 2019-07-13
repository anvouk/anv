/*
 * The MIT License
 *
 * Copyright 2019 Andrea Vouk.
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
    anv_hhoh (windows only)
--------------------------------------------------------------------------------
  
  open, close and interchange between C file descriptors, windows's HANDLEs and 
  C FILEs.
  
  Handy Handler Of Handles (aka hhoh) has 2 main purposes:
    1. hide boilerplate code
    2. create a unifyed and cleaner interface

  simple example:

    #define ANV_HHOH_IMPLEMENTATION
    #include <anv_hhoh.h>

    #include <windows.h>
    #include <fileapi.h>
    #include <tchar.h>

    int main(void)
    {
        ANV_HHOH_EXP file_handle;

        if (!anv_hhoh_open_file(&file_handle, TEXT("my_file.txt"), TEXT("r"))) {
            // could not open the file
        }

        // to C file descriptor
        if (!anv_hhoh_file_to_cfd(&file_handle)) {
            // conversion failure
        }

        // to win32 api handle
        if (!anv_hhoh_cfd_to_win32(&file_handle)) {
            // conversion failure
        }
        
        long read_count;
        TCHAR text[2048];
        if (!ReadFile(file_handle.handle, &text, 2048, &read_count, NULL)) {
            // error checking ...
        }

        _tprintf(TEXT("%s\n"), text);

        anv_hhoh_close_auto(&file_handle); // error checking (?)
    }

------------------------------------------------------------------------------*/

#ifndef ANV_HHOH_H
#define ANV_HHOH_H

#ifndef _WIN32
#  error Only available on Windows!
#endif

#pragma once

#include <windows.h>
#include <stdio.h>

#ifndef ANV_HHOH_EXP
#  define ANV_HHOH_EXP extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ANV_HHOH_HANDLE_INVALID	0
#define ANV_HHOH_HANDLE_C_FD	1
#define ANV_HHOH_HANDLE_WIN32	2
#define ANV_HHOH_HANDLE_FILE	3

typedef struct ANV_HANDLE {
	union {
		int fd;
		HANDLE handle;
		FILE *file;
	};
	int current;
} ANV_HANDLE;

/*------------------------------------------------------------------------------
	open - close
------------------------------------------------------------------------------*/

ANV_HHOH_EXP BOOL anv_hhoh_open_cfd(ANV_HANDLE *hd, const TCHAR *filename, int mode);
ANV_HHOH_EXP BOOL anv_hhoh_open_win32(ANV_HANDLE *hd, const TCHAR *filename, DWORD mode, BOOL shared);
ANV_HHOH_EXP BOOL anv_hhoh_open_file(ANV_HANDLE *hd, const TCHAR *filename, const TCHAR *mode);

ANV_HHOH_EXP BOOL anv_hhoh_close_cfd(ANV_HANDLE *hd);
ANV_HHOH_EXP BOOL anv_hhoh_close_win32(ANV_HANDLE *hd);
ANV_HHOH_EXP BOOL anv_hhoh_close_file(ANV_HANDLE *hd);

ANV_HHOH_EXP BOOL anv_hhoh_close_auto(ANV_HANDLE *hd);

/*------------------------------------------------------------------------------
	conversions
------------------------------------------------------------------------------*/

ANV_HHOH_EXP BOOL anv_hhoh_file_to_cfd(ANV_HANDLE *hd);
ANV_HHOH_EXP BOOL anv_hhoh_cfd_to_file(ANV_HANDLE *hd, const TCHAR *mode);

ANV_HHOH_EXP BOOL anv_hhoh_win32_to_cfd(ANV_HANDLE *hd, int flags);
ANV_HHOH_EXP BOOL anv_hhoh_cfd_to_win32(ANV_HANDLE *hd);

#ifdef __cplusplus
}
#endif

#ifdef ANV_HHOH_IMPLEMENTATION

#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#ifndef anv_hhoh__assert
#  include <assert.h>
#  define anv_hhoh__assert(x) assert(x)
#endif

/*------------------------------------------------------------------------------
    open functions
------------------------------------------------------------------------------*/

// _O_CREAT, _O_TEXT, ...
BOOL
anv_hhoh_open_cfd(ANV_HANDLE *hd, const TCHAR *filename, int mode)
{
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/sopen-s-wsopen-s
    errno_t err = _tsopen_s(
        &hd->fd,
        filename,
        mode,
        _SH_DENYNO,
        _S_IREAD | _S_IWRITE
    );
    if (err != 0) {
        hd->current = ANV_HHOH_HANDLE_INVALID;
        return FALSE;
    }
    hd->current = ANV_HHOH_HANDLE_C_FD;
    return TRUE;
}

// CREATE_NEW, OPEN_EXISTING, ...
BOOL
anv_hhoh_open_win32(ANV_HANDLE *hd, const TCHAR *filename, DWORD mode, BOOL shared)
{
    // https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea
    HANDLE hfile = CreateFile(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        shared ? FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE : 0,
        NULL,
        mode,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hfile == INVALID_HANDLE_VALUE) {
        hd->current = ANV_HHOH_HANDLE_INVALID;
        return FALSE;
    }
    hd->handle = hfile;
    hd->current = ANV_HHOH_HANDLE_WIN32;
    return TRUE;
}

// "r", "w", ...
BOOL
anv_hhoh_open_file(ANV_HANDLE *hd, const TCHAR *filename, const TCHAR *mode)
{
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen?f1url=https%3A%2F%2Fmsdn.microsoft.com%2Fquery%2Fdev15.query%3FappId%3DDev15IDEF1%26l%3DEN-US%26k%3Dk(TCHAR%2F_tfopen)%3Bk(_tfopen)%3Bk(DevLang-C%2B%2B)%3Bk(TargetOS-Windows)%26rd%3Dtrue%26f%3D255%26MSPPError%3D-2147217396
    FILE *file = _tfopen(filename, mode);
    if (!file) {
        hd->current = ANV_HHOH_HANDLE_INVALID;
        return FALSE;
    }
    hd->file = file;
    hd->current = ANV_HHOH_HANDLE_FILE;
    return TRUE;
}

/*------------------------------------------------------------------------------
    close Functions
------------------------------------------------------------------------------*/

BOOL
anv_hhoh_close_cfd(ANV_HANDLE *hd)
{
    anv_hhoh__assert(hd->current == ANV_HHOH_HANDLE_C_FD);
    hd->current = ANV_HHOH_HANDLE_INVALID;
    return _close(hd->fd) == 0;
}

BOOL
anv_hhoh_close_win32(ANV_HANDLE *hd)
{
    anv_hhoh__assert(hd->current == ANV_HHOH_HANDLE_WIN32);
    hd->current = ANV_HHOH_HANDLE_INVALID;
    return CloseHandle(hd->handle);
}

BOOL
anv_hhoh_close_file(ANV_HANDLE *hd)
{
    anv_hhoh__assert(hd->current == ANV_HHOH_HANDLE_FILE);
    hd->current = ANV_HHOH_HANDLE_INVALID;
    return fclose(hd->file) == 0;
}

BOOL
anv_hhoh_close_auto(ANV_HANDLE *hd)
{
    switch (hd->current) {
        case ANV_HHOH_HANDLE_C_FD:
            return anv_hhoh_close_cfd(hd);
        case ANV_HHOH_HANDLE_WIN32:
            return anv_hhoh_close_win32(hd);
        case ANV_HHOH_HANDLE_FILE:
            return anv_hhoh_close_file(hd);
        default:
            anv_hhoh__assert(0);
            return FALSE;
    }
}

/*------------------------------------------------------------------------------
    conversions
------------------------------------------------------------------------------*/

BOOL
anv_hhoh_file_to_cfd(ANV_HANDLE *hd)
{
    hd->fd = _fileno(hd->file);
    if (hd->fd == -1) {
        hd->current = ANV_HHOH_HANDLE_INVALID;
        return FALSE;
    }
    hd->current = ANV_HHOH_HANDLE_C_FD;
    return TRUE;
}

// "r", "w", ...
BOOL
anv_hhoh_cfd_to_file(ANV_HANDLE *hd, const TCHAR *mode)
{
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fdopen-wfdopen?f1url=https%3A%2F%2Fmsdn.microsoft.com%2Fquery%2Fdev15.query%3FappId%3DDev15IDEF1%26l%3DEN-US%26k%3Dk(TCHAR%2F_tfdopen)%3Bk(_tfdopen)%3Bk(DevLang-C%2B%2B)%3Bk(TargetOS-Windows)%26rd%3Dtrue%26f%3D255%26MSPPError%3D-2147217396
    hd->file = _tfdopen(hd->fd, mode);
    if (!hd->file) {
        hd->current = ANV_HHOH_HANDLE_INVALID;
        return FALSE;
    }
    hd->current = ANV_HHOH_HANDLE_FILE;
    return TRUE;
}

// _O_APPEND, _O_RDONLY, _O_TEXT, _O_WTEXT
BOOL
anv_hhoh_win32_to_cfd(ANV_HANDLE *hd, int flags)
{
    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/open-osfhandle?f1url=https%3A%2F%2Fmsdn.microsoft.com%2Fquery%2Fdev15.query%3FappId%3DDev15IDEF1%26l%3DEN-US%26k%3Dk(CORECRT_IO%2F_open_osfhandle)%3Bk(_open_osfhandle)%3Bk(DevLang-C%2B%2B)%3Bk(TargetOS-Windows)%26rd%3Dtrue
    hd->fd = _open_osfhandle((intptr_t)hd->handle, flags);
    if (hd->fd == -1) {
        hd->current = ANV_HHOH_HANDLE_INVALID;
        return FALSE;
    }
    hd->current = ANV_HHOH_HANDLE_C_FD;
    return TRUE;
}

BOOL
anv_hhoh_cfd_to_win32(ANV_HANDLE *hd)
{
    hd->handle = (HANDLE)_get_osfhandle(hd->fd);
    if (hd->handle == INVALID_HANDLE_VALUE) {
        hd->current = ANV_HHOH_HANDLE_INVALID;
        return FALSE;
    }
    hd->current = ANV_HHOH_HANDLE_WIN32;
    return TRUE;
}

#endif

#endif /* ANV_HHOH_H */
