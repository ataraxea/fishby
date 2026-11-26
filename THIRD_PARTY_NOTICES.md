# Third-Party Notices

fishby is licensed under the MIT License.

This project uses third-party libraries and compiler
runtime components that are distributed under their
respective licenses.

## ncurses

Used for terminal rendering on Linux.

- Project: ncurses
- License: MIT/X11-style license
- Website: https://invisible-island.net/ncurses/
- Licensing: https://invisible-island.net/ncurses/ncurses-license.html

Linux builds normally use the curses
implementation provided by the target system.

## PDCurses

Used for terminal rendering on Windows.

- Project: PDCurses
- Version: 3.9
- Backend: Windows Console (wincon)
- License: Public domain for the core and wincon code
- Repository: https://github.com/wmcbrine/PDCurses

PDCurses is statically linked into Windows builds.

Individual upstream files may have separate licensing
terms.

## MinGW-w64 Runtime

Windows builds use the MinGW-w64 toolchain.

- Project: MinGW-w64
- Website: https://www.mingw-w64.org/
- Repository: https://github.com/mingw-w64/mingw-w64

Windows executables may contain statically linked
runtime components distributed under their respective
licenses.

The applicable runtime license notices must accompany
distributed binaries where required.

## GCC Runtime Library

GCC-built executables may include GCC runtime
components.

Applicable components are distributed under the
GNU General Public License with the GCC Runtime
Library Exception.

- Project: GCC
- Website: https://gcc.gnu.org/
- Runtime Library Exception:
  https://www.gnu.org/licenses/gcc-exception-3.1.html

## Distribution

The MIT License applies to the original fishby code.

Third-party components retain their respective
copyright notices and licensing terms.

Binary release packages include applicable third-party
license notices for the components they distribute.
