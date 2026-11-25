#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "state.h"

#define PATH_CAPACITY 4096
#define SESSION_ID 0

static HANDLE directory_handle = INVALID_HANDLE_VALUE;
static HANDLE lock_handle = INVALID_HANDLE_VALUE;

static wchar_t storage_path[PATH_CAPACITY];

static int session_valid(int session)
{
    return session == SESSION_ID &&
           directory_handle != INVALID_HANDLE_VALUE &&
           lock_handle != INVALID_HANDLE_VALUE;
}

static int build_path(
    wchar_t *output,
    size_t capacity,
    const wchar_t *name
)
{
    int n = swprintf(
        output,
        capacity,
        L"%ls\\%ls",
        storage_path,
        name
    );

    if (n < 0 || (size_t)n >= capacity)
        return -1;

    return 0;
}

static int validate_file(HANDLE handle)
{
    BY_HANDLE_FILE_INFORMATION info;

    if (GetFileType(handle) != FILE_TYPE_DISK)
        return -1;

    if (!GetFileInformationByHandle(handle, &info))
        return -1;

    if (info.dwFileAttributes &
        (FILE_ATTRIBUTE_DIRECTORY |
         FILE_ATTRIBUTE_REPARSE_POINT)) {
        return -1;
    }

    if (info.nNumberOfLinks != 1)
        return -1;

    return 0;
}

static int validate_directory(HANDLE handle)
{
    BY_HANDLE_FILE_INFORMATION info;

    if (!GetFileInformationByHandle(handle, &info))
        return -1;

    if (!(info.dwFileAttributes &
          FILE_ATTRIBUTE_DIRECTORY)) {
        return -1;
    }

    if (info.dwFileAttributes &
        FILE_ATTRIBUTE_REPARSE_POINT) {
        return -1;
    }

    return 0;
}

int state_open(void)
{
    if (directory_handle != INVALID_HANDLE_VALUE ||
        lock_handle != INVALID_HANDLE_VALUE) {
        return -1;
    }

    wchar_t base[PATH_CAPACITY];

    DWORD length = GetEnvironmentVariableW(
        L"LOCALAPPDATA",
        base,
        PATH_CAPACITY
    );

    if (length == 0 || length >= PATH_CAPACITY)
        return -1;

    /* Require an absolute drive-based path */
    if (length < 3 ||
        !((base[0] >= L'A' && base[0] <= L'Z') ||
          (base[0] >= L'a' && base[0] <= L'z')) ||
        base[1] != L':' ||
        base[2] != L'\\') {
        return -1;
    }

    int n = swprintf(
        storage_path,
        PATH_CAPACITY,
        L"%ls\\fishby",
        base
    );

    if (n < 0 || n >= PATH_CAPACITY)
        return -1;

    if (!CreateDirectoryW(storage_path, NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
            return -1;
    }

    directory_handle = CreateFileW(
        storage_path,
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS |
        FILE_FLAG_OPEN_REPARSE_POINT,
        NULL
    );

    if (directory_handle == INVALID_HANDLE_VALUE)
        return -1;

    if (validate_directory(directory_handle) < 0)
        goto fail;

    wchar_t path[PATH_CAPACITY];

    if (build_path(
            path,
            PATH_CAPACITY,
            L"pet.lock"
        ) < 0) {
        goto fail;
    }

    /* Deny sharing while the storage session is active */
    lock_handle = CreateFileW(
        path,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_OPEN_REPARSE_POINT,
        NULL
    );

    if (lock_handle == INVALID_HANDLE_VALUE)
        goto fail;

    if (validate_file(lock_handle) < 0)
        goto fail;

    return SESSION_ID;

fail:
    if (lock_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(lock_handle);
        lock_handle = INVALID_HANDLE_VALUE;
    }

    if (directory_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(directory_handle);
        directory_handle = INVALID_HANDLE_VALUE;
    }

    return -1;
}

void state_close(int session)
{
    if (!session_valid(session))
        return;

    CloseHandle(directory_handle);
    directory_handle = INVALID_HANDLE_VALUE;

    CloseHandle(lock_handle);
    lock_handle = INVALID_HANDLE_VALUE;
}

static int parse_number(
    char **p,
    uint64_t *value
)
{
    if (**p < '0' || **p > '9')
        return -1;

    char *end;

    errno = 0;

    uintmax_t n = strtoumax(*p, &end, 10);

    if (errno == ERANGE ||
        n > UINT64_MAX ||
        *end != '\n')
        return -1;

    *p = end + 1;
    *value = (uint64_t)n;

    return 0;
}

int state_load(int session, Pet *pet)
{
    if (!session_valid(session))
        return -1;

    wchar_t path[PATH_CAPACITY];

    if (build_path(
            path,
            PATH_CAPACITY,
            L"pet.state"
        ) < 0) {
        return -1;
    }

    HANDLE file = CreateFileW(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        return GetLastError() == ERROR_FILE_NOT_FOUND
            ? 0 : -1;
    }

    char buffer[128];
    size_t used = 0;

    if (validate_file(file) < 0)
        goto fail;

    /* Read a bounded amount of data */
    for (;;) {
        if (used == sizeof(buffer) - 1)
            goto fail;

        DWORD count = 0;

        if (!ReadFile(
                file,
                buffer + used,
                (DWORD)(sizeof(buffer) - 1 - used),
                &count,
                NULL
            )) {
            goto fail;
        }

        if (count == 0)
            break;

        used += count;
    }

    if (memchr(buffer, '\0', used))
        goto fail;

    buffer[used] = '\0';

    if (strncmp(buffer, "FISHBY1\n", 8) != 0)
        goto fail;

    char *p = buffer + 8;

    uint64_t hunger;
    uint64_t eaten;

    if (parse_number(&p, &hunger) < 0 ||
        hunger > 100000 ||
        parse_number(&p, &eaten) < 0 ||
        *p != '\0') {
        goto fail;
    }

    CloseHandle(file);

    /* Apply state only after successful validation */
    pet->hunger = (float)hunger / 1000.0f;
    pet->eaten = eaten;
    pet->happy_timer = 0.0f;

    pet_update(pet, 0.0f);

    return 1;

fail:
    CloseHandle(file);
    return -1;
}

int state_save(int session, const Pet *pet)
{
    if (!session_valid(session))
        return -1;

    if (!isfinite(pet->hunger) ||
        pet->hunger < 0.0f ||
        pet->hunger > 100.0f) {
        return -1;
    }

    char buffer[128];

    int length = snprintf(
        buffer,
        sizeof(buffer),
        "FISHBY1\n%lu\n%" PRIu64 "\n",
        (unsigned long)lroundf(
            pet->hunger * 1000.0f
        ),
        pet->eaten
    );

    if (length < 0 ||
        (size_t)length >= sizeof(buffer)) {
        return -1;
    }

    wchar_t destination[PATH_CAPACITY];
    wchar_t temporary[PATH_CAPACITY];
    wchar_t name[64];

    if (build_path(
            destination,
            PATH_CAPACITY,
            L"pet.state"
        ) < 0) {
        return -1;
    }

    HANDLE file = INVALID_HANDLE_VALUE;

    /* Exclusively create a temporary file */
    for (unsigned int i = 0; i < 128; i++) {
        int n = swprintf(
            name,
            sizeof(name) / sizeof(name[0]),
            L".pet.%lu.%u.tmp",
            (unsigned long)GetCurrentProcessId(),
            i
        );

        if (n < 0 ||
            (size_t)n >=
            sizeof(name) / sizeof(name[0])) {
            return -1;
        }

        if (build_path(
                temporary,
                PATH_CAPACITY,
                name
            ) < 0) {
            return -1;
        }

        file = CreateFileW(
            temporary,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL |
            FILE_FLAG_OPEN_REPARSE_POINT,
            NULL
        );

        if (file != INVALID_HANDLE_VALUE)
            break;

        DWORD error = GetLastError();

        if (error != ERROR_FILE_EXISTS &&
            error != ERROR_ALREADY_EXISTS) {
            return -1;
        }
    }

    if (file == INVALID_HANDLE_VALUE)
        return -1;

    size_t used = 0;
    int ok = 1;

    /* Handle partial writes */
    while (used < (size_t)length) {
        DWORD written = 0;

        if (!WriteFile(
                file,
                buffer + used,
                (DWORD)((size_t)length - used),
                &written,
                NULL
            ) || written == 0) {

            ok = 0;
            break;
        }

        used += written;
    }

    if (ok && !FlushFileBuffers(file))
        ok = 0;

    if (!CloseHandle(file))
        ok = 0;

    if (!ok) {
        DeleteFileW(temporary);
        return -1;
    }

    /* Replace the previous save */
    if (!MoveFileExW(
            temporary,
            destination,
            MOVEFILE_REPLACE_EXISTING |
            MOVEFILE_WRITE_THROUGH
        )) {

        DeleteFileW(temporary);
        return -1;
    }

    return 0;
}
