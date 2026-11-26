# Changelog

All notable changes to fishby will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/).

## [1.0.0] - 2026-11-25

### Added

- Interactive ASCII terminal aquarium.
- Organic fish movement and tail animations.
- Animated bubbles, water, sand, and aquatic plants.
- Interactive feeding with simulated food particles.
- Hunger and mood simulation.
- Persistent hunger and food consumption statistics.
- Custom fish names.
- Configurable frame rate from 10 to 120 FPS.
- Color and monochrome modes.
- Session-only mode without persistent storage.
- Command-line help and version information.
- Fixed-timestep simulation.
- Monotonic platform-specific timing.
- Adaptive terminal dimensions.
- Graceful shutdown and terminal restoration.
- Versioned persistent state format.
- Portable 64-bit food consumption counter.
- Concurrent storage protection.
- POSIX and Windows storage backends.
- ncurses and PDCurses terminal support.
- Cross-platform CMake build configuration.
- MinGW-w64 cross-compilation toolchain.
- Linux development Makefile.
- Platform-specific compiler hardening.
- AddressSanitizer and UndefinedBehaviorSanitizer builds.
- Clang Static Analyzer and Cppcheck integration.
- Automated unit and regression tests.
- Persistent storage security tests.
- GitHub Actions build and test workflows.
- Downloadable CI build artifacts.
- Project documentation and contribution guidelines.
- Security policy and MIT license.

### Fixed

- Fish movement and animation edge cases.
- Food consumption counter overflow.
- Simulation timing inconsistencies.
- Terminal restoration after interruption.
- Concurrent persistent state modification.
- Platform-dependent persistent counter width.
- Windows console color visibility.
- Windows terminal resize handling.
- Missing trailing newlines in source files.

### Security

- Bounded persistent state parsing.
- Strict state format and numeric validation.
- Restrictive POSIX storage permissions.
- POSIX file ownership verification.
- Symbolic-link checks for POSIX storage.
- Reparse-point checks for Windows storage.
- Exclusive temporary-file creation.
- Temporary-file replacement for persistent updates.
- Explicit flushing of saved state data.
- Compiler-level exploit mitigation options.
- Automated storage security regression tests.

### Known Limitations

- Terminal colors depend on the user's color palette.
- Frame-rate accuracy depends on platform scheduling.
- Windows storage protections differ from POSIX.
- Automated tests do not cover all interactive behavior.
- The project has not undergone an independent security audit.
