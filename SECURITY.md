# Security Policy

## Supported Versions

| Version | Security support |
|---------|------------------|
| 1.0.x   | Supported        |
| main    | Supported        |
| < 1.0.0 | Not supported    |

Users are encouraged to use the latest stable release.

This policy will be updated when the first stable
release is published.

## Security Model

fishby is a local terminal application written in C.

The application does not require:

- Network access
- User accounts
- External services
- Elevated privileges
- Telemetry

fishby should be run as a regular, unprivileged user.

## Security Measures

The project includes several measures intended to
reduce common security and reliability risks.

### Input Validation

- Command-line argument validation
- Bounded state-file parsing
- State format and version validation
- Numeric range validation
- Saturating persistent food counter

### Persistent Storage

- Restrictive POSIX file and directory permissions
- POSIX file ownership checks
- Symbolic-link protection for POSIX state files
- Reparse-point checks for Windows state files
- Concurrent access protection
- Exclusive temporary-file creation
- Temporary-file replacement for state updates
- Explicit flushing of saved state data

### Compiler Hardening

Depending on the target platform and compiler,
release builds may include:

- Stack protector
- Stack clash protection
- Position-independent executable (PIE)
- Full RELRO
- Non-executable stack
- Fortified standard library operations
- Windows dynamic base support
- Windows NX compatibility
- High-entropy address-space randomization

The available protections vary across platforms
and build configurations.

### Development Checks

- AddressSanitizer (ASan)
- UndefinedBehaviorSanitizer (UBSan)
- Clang Static Analyzer
- Cppcheck
- Automated unit and regression tests
- Persistent storage security tests
- Cross-platform continuous integration

## Security Limitations

These measures reduce risk but do not guarantee
that fishby is free from vulnerabilities.

The project has not undergone an independent
security audit.

Additional limitations include:

- Windows storage security differs from POSIX
  ownership and permission checks.
- Storage checks assume trusted parent directories.
- Windows directory ACLs are not independently audited.
- State-file locking is intended for cooperating
  fishby instances, not arbitrary external programs.
- Filesystem durability guarantees vary by platform.
- Automated testing does not cover every possible
  terminal or filesystem behavior.
- Third-party libraries and operating system
  components have their own security properties.

fishby is not intended to store sensitive information
or provide a security boundary against privileged
local attackers.

## Reporting a Vulnerability

If you discover a potential security vulnerability,
please avoid publishing exploit details in a public issue.

Use GitHub private vulnerability reporting if it is
available for this repository:

https://github.com/ataraxea/fishby/security

If private vulnerability reporting is unavailable,
contact the maintainer through an available private
communication channel.

Please include:

- A description of the vulnerability
- The affected version or commit
- Operating system and architecture
- Steps required to reproduce the issue
- Potential security impact
- Relevant logs or minimal test cases
- A proposed fix, if available

Do not include credentials, private files, or other
sensitive information in public reports.

## Responsible Disclosure

Please allow the maintainer a reasonable opportunity
to investigate and address reported vulnerabilities
before publicly disclosing technical details.

Reports will be reviewed on a best-effort basis.

As fishby is a personal open-source project, no
specific response or remediation timeframe is
guaranteed.

Thank you for helping keep fishby safe.
