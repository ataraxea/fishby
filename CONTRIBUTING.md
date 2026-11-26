# Contributing to fishby

Thank you for your interest in contributing to fishby!

fishby is a lightweight, cross-platform terminal
aquarium written in C.

Contributions of all sizes are welcome.

## Getting Started

1. Fork the repository on GitHub.
2. Clone your fork.
3. Create a new branch for your changes.
4. Install development dependencies.
5. Make your changes and add tests.
6. Open a pull request.

## Development Setup

fishby requires a C11-compatible compiler and a
supported curses implementation.

Clone the repository:

```sh
git clone https://github.com/ataraxea/fishby.git
cd fishby
```

### Linux

Install the required dependencies on Arch Linux:

```sh
sudo pacman -S --needed base-devel ncurses cmake clang cppcheck
```

Build the application:

```sh
make
```

Run locally:

```sh
./fishby --no-save
```

Run all development checks:

```sh
make check
```

### Cross-platform Development

Use CMake for Linux and Windows builds.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Windows builds require MinGW-w64 and PDCurses.
The GitHub Actions workflow provides a reference
for configuring the Windows build environment.

## Development Guidelines

- Keep changes focused and easy to review.
- Follow the existing C11 coding style.
- Use descriptive names for functions and variables.
- Keep platform-specific code isolated.
- Avoid unnecessary dependencies.
- Check return values from system calls.
- Validate external input before processing it.
- Prevent integer overflow and out-of-bounds access.
- Preserve existing state-file compatibility where practical.
- Avoid weakening storage security or compiler hardening.
- Keep rendering independent of simulation timing.
- Preserve terminal state when exiting.
- Add or update tests when changing behavior.
- Update documentation when necessary.

Keep fishby lightweight. New features should improve
the aquarium without introducing unnecessary complexity.

## Testing

Run the complete Linux development checks:

```sh
make check
```

Individual checks are also available:

```sh
make test
make sanitize
make analyze
make lint
```

Run the sanitizer build interactively:

```sh
./fishby-san --no-save
```

For CMake builds:

```sh
ctest --test-dir build --output-on-failure
```

Changes affecting rendering should also be tested manually.

Verify the following where applicable:

- Terminal resizing
- Small terminal dimensions
- Color and monochrome modes
- Interactive feeding
- Frame-rate configuration
- Normal exit and signal handling
- Persistent and session-only modes
- Compatibility across supported platforms

Use temporary storage or session-only mode when testing
changes that could affect existing pet statistics.

## Commit Messages

We use Conventional Commits.

Examples:

- `feat: introduce a new fish animation`
- `fix: handle small terminal dimensions`
- `refactor: simplify food particle simulation`
- `test: cover malformed state files`
- `build: improve cross-platform configuration`
- `ci: update GitHub Actions workflow`
- `docs: improve installation instructions`

## Pull Requests

Before opening a pull request:

- Make sure your code works locally.
- Run the relevant automated tests.
- Check for compiler warnings.
- Add tests for bug fixes or new functionality.
- Update documentation when necessary.
- Explain what changed and why.
- Avoid unrelated changes in the same pull request.

GitHub Actions is configured to build and test fishby
on Linux and Windows.

A pull request may require review and additional
changes before merging.

## Reporting Bugs

When reporting a bug, please include:

- Operating system and architecture
- Terminal emulator
- fishby version (`fishby --version`)
- Compiler and build method, if built from source
- Command-line arguments
- Expected behavior
- Actual behavior
- Relevant error messages

For visual issues, screenshots or short recordings
are especially helpful.

Review logs and screenshots before uploading them.
Remove any sensitive information.

## Feature Requests

Before implementing a major feature, consider opening
an issue to discuss its behavior and scope.

fishby is intentionally small. Not every feature
needs to become part of the application.

## Security Issues

Report potential security vulnerabilities privately.

See [SECURITY.md](SECURITY.md) for reporting guidelines.

## Code of Conduct

Please communicate respectfully and keep technical
discussions constructive.

Thank you for helping improve fishby!
