#define _POSIX_C_SOURCE 200809L

#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <unistd.h>

#include "pet.h"
#include "state.h"

static void write_fixture(int dir, const char *data)
{
    int fd = openat(
        dir,
        "pet.state",
        O_WRONLY | O_CREAT | O_TRUNC |
        O_NOFOLLOW | O_CLOEXEC,
        0600
    );

    assert(fd >= 0);

    size_t length = strlen(data);
    size_t offset = 0;

    while (offset < length) {
        ssize_t n = write(
            fd,
            data + offset,
            length - offset
        );

        if (n < 0 && errno == EINTR)
            continue;

        assert(n > 0);
        offset += (size_t)n;
    }

    assert(close(fd) == 0);
}

static void test_roundtrip(int dir)
{
    Pet original;
    pet_init(&original);

    original.hunger = 42.125f;
    original.eaten = 123;

    assert(state_save(dir, &original) == 0);

    Pet restored;
    pet_init(&restored);

    assert(state_load(dir, &restored) == 1);

    assert(fabsf(
        restored.hunger - original.hunger
    ) < 0.001f);

    assert(restored.eaten == original.eaten);

    puts("PASS: state roundtrip");
}

static void test_permissions(int dir)
{
    struct stat st;

    assert(fstat(dir, &st) == 0);
    assert((st.st_mode & 0777) == 0700);

    assert(fstatat(
        dir, "pet.state", &st,
        AT_SYMLINK_NOFOLLOW
    ) == 0);

    assert(S_ISREG(st.st_mode));
    assert((st.st_mode & 0777) == 0600);

    assert(fstatat(
        dir, "pet.lock", &st,
        AT_SYMLINK_NOFOLLOW
    ) == 0);

    assert(S_ISREG(st.st_mode));
    assert((st.st_mode & 0777) == 0600);

    puts("PASS: state permissions");
}

static void test_invalid_states(int dir)
{
    const char *fixtures[] = {
        "",
        "INVALID\n30000\n3\n",
        "FISHBY2\n30000\n3\n",
        "FISHBY1\n100001\n3\n",
        "FISHBY1\n-1000\n3\n",
        "FISHBY1\n30000\n-1\n",
        "FISHBY1\n30000\n",
        "FISHBY1\n30000\n3\nextra\n",
        "FISHBY1\n30000\n999999999999999999999999999999\n"
    };

    size_t count =
        sizeof(fixtures) / sizeof(fixtures[0]);

    for (size_t i = 0; i < count; i++) {
        write_fixture(dir, fixtures[i]);

        Pet pet;
        pet_init(&pet);

        assert(state_load(dir, &pet) < 0);

        /* Invalid data must not modify pet state */
        assert(pet.hunger == 30.0f);
        assert(pet.eaten == 0);
    }

    puts("PASS: malformed state rejection");
}

static void test_state_symlink(int dir)
{
    assert(unlinkat(dir, "pet.state", 0) == 0);

    assert(symlinkat(
        "/etc/passwd",
        dir,
        "pet.state"
    ) == 0);

    Pet pet;
    pet_init(&pet);

    assert(state_load(dir, &pet) < 0);

    struct stat st;

    assert(fstatat(
        dir,
        "pet.state",
        &st,
        AT_SYMLINK_NOFOLLOW
    ) == 0);

    assert(S_ISLNK(st.st_mode));

    assert(unlinkat(dir, "pet.state", 0) == 0);

    puts("PASS: state symlink rejection");
}

static void test_concurrent_lock(
    const char *program
)
{
    pid_t pid = fork();

    assert(pid >= 0);

    if (pid == 0) {
        execl(
            program,
            program,
            "--probe-lock",
            (char *)NULL
        );

        _exit(127);
    }

    int status;
    pid_t result;

    do {
        result = waitpid(pid, &status, 0);
    } while (result < 0 && errno == EINTR);

    assert(result == pid);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 0);

    puts("PASS: concurrent lock rejection");
}

int main(int argc, char **argv)
{
    /* Probe storage from an independent process */
    if (argc == 2 &&
        strcmp(argv[1], "--probe-lock") == 0) {

        int dir = state_open();

        if (dir >= 0) {
            state_close(dir);
            return EXIT_FAILURE;
        }

        return errno == EWOULDBLOCK ||
               errno == EAGAIN
            ? EXIT_SUCCESS
            : EXIT_FAILURE;
    }

    umask(0077);

    char temp[] = "/tmp/fishby-test-XXXXXX";

    assert(mkdtemp(temp) != NULL);
    assert(setenv("XDG_DATA_HOME", temp, 1) == 0);

    int dir = state_open();

    assert(dir >= 0);

    Pet pet;
    pet_init(&pet);

    assert(state_load(dir, &pet) == 0);

    puts("PASS: missing state handling");

    test_roundtrip(dir);
    test_permissions(dir);
    test_invalid_states(dir);
    test_state_symlink(dir);
    test_concurrent_lock(argv[0]);

    /* Verify explicit storage session cleanup */
    state_close(dir);

    assert(state_load(dir, &pet) < 0);
    assert(state_save(dir, &pet) < 0);

    /* Storage must be available again */
    dir = state_open();

    assert(dir >= 0);

    puts("PASS: storage session reopened");

    test_concurrent_lock(argv[0]);

    assert(state_save(dir, &pet) == 0);

    assert(unlinkat(dir, "pet.state", 0) == 0);

    /*
     * All competing processes have exited.
     * Remove the lock only during fixture cleanup.
     */
    assert(unlinkat(dir, "pet.lock", 0) == 0);
    state_close(dir);

    char path[256];

    int n = snprintf(
        path,
        sizeof(path),
        "%s/fishby",
        temp
    );

    assert(n > 0 && (size_t)n < sizeof(path));

    assert(rmdir(path) == 0);
    assert(rmdir(temp) == 0);

    puts("PASS: all state security tests");

    return EXIT_SUCCESS;
}
