# SlyChess
SlyChess is a UCI chess engine coded in C++.

## Playing Against SlyChess

SlyChess is a command-line engine and does not include a built-in graphical interface. You can play against it in two ways:

- **Online via Lichess:** Challenge the bot directly on its [Lichess page][lichess-link] *(note: the bot is not always online)*.
- **Locally via a Chess GUI:** Download the engine and load it into any UCI-compatible interface, such as [En Croissant][en-croissant-link].

## Installation

1. Download the latest pre-compiled executable for your operating system from the [Releases][releases-link] page.
2. Add the binary to your preferred UCI-compatible GUI (my personal favorite being [En Croissant][en-croissant-link]).

> **Note:** Because binaries are unsigned, Windows or macOS may display a security warning on first run. On macOS, you can bypass this by running `xattr -d com.apple.quarantine ./SlyChess` in your terminal.

## Contributing
Issues and PRs are welcome! I'll try my best to go through them.

> [!IMPORTANT]
> **AI-Generated Code:** Maintaining this project takes time, and reviewing unverified AI output slows everyone down. If you use an LLM, treat it as a drafting tool. Please review, test, and thoroughly understand every line *before* opening a Pull Request. PRs containing AI hallucinations, broken logic, or bloated boilerplate will be closed without review.

### Building from Source

**Prerequisites:**
- A C++20-compatible compiler (tested and verified with `MSVC` and `g++`; `clang++` should also work)
- [CMake](https://cmake.org/) (3.16+)
- [Ninja](https://ninja-build.org/)
- Git

**Build Steps:**
```bash
git clone https://github.com/JavaCoder5/SlyChess.git
cd SlyChess
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## License
SlyChess is licensed under the GNU General Public License v3.0 (GPL v3).
See the [LICENSE.md][license-link] file for more details.

[lichess-link]: https://lichess.org/@/slychessbot
[license-link]: https://github.com/JavaCoder5/SlyChess/blob/main/LICENSE.md
[en-croissant-link]: https://encroissant.org/
[releases-link]: https://github.com/JavaCoder5/SlyChess/releases
