Below is an updated `README.md` adjusted for the C++ port (cSpy):

---

# cSpy

cSpy is a lightweight, modern C++ tool inspired by [chaos](https://chaos.projectdiscovery.io/) from ProjectDiscovery.io and ported from [chaospy](https://github.com/zorox0x/chaospy/). It fetches the latest bug bounty program data and lets you list and download programs directly from the chaos-data index—now built in C++ for speed and efficiency!

<p align="center">
  <img src="https://github.com/zorox0x/chaospy/blob/master/image.png" alt="cSpy banner" style="max-width:600px;">
  </br>
  <a href="https://en.cppreference.com/w/">
    <img src="https://img.shields.io/badge/made%20with-C++17-blue.svg" alt="made with C++17">
  </a>
  <a href="https://github.com/algenpartridge/cSpy/issues">
    <img src="https://img.shields.io/github/issues/aldenpartridge/cSpy.svg" alt="GitHub issues">
  </a>
  <a href="https://twitter.com/intent/follow?screen_name=0xkmac">
    <img src="https://img.shields.io/twitter/follow/zor0x0x?style=social&logo=twitter" alt="Follow on Twitter">
  </a>
</p>

---

## Installation

### Linux / macOS

1. **Clone the repository:**

   ```bash
   git clone https://github.com/aldenpartridge/cSpy.git
   cd cSpy
   ```

2. **Build the project:**

   Ensure you have a C++17-compatible compiler installed (e.g., `g++` 7+ or `clang++` 6+), along with CMake.

   ```bash
   cmake .
   make
   ```

3. **(Optional) Install Dependencies:**

   cSpy uses cURL and the [nlohmann/json](https://github.com/nlohmann/json) library. These dependencies are usually available via your package manager or bundled in the project. Make sure cURL is installed:
   
   ```bash
   sudo apt-get install libcurl4-openssl-dev   # Debian/Ubuntu
   sudo dnf install libcurl-devel              # Fedora
   ```

---

## Usage

After building, the cSpy binary is located in your `build` directory. Make it executable (if not already) and run with the appropriate options:

```bash
./cSpy --help
```

Example usage:

- **List all programs:**

  ```bash
  ./cSpy --list
  ```

- **Download all HackerOne programs:**

  ```bash
  ./cSpy --download-hackerone
  ```

- **Download a specific program:**

  ```bash
  ./cSpy -download <program_name>
  ```

### Available Options

```text
  -h, --help            Show this help message and exit
  -list                 List all programs
  --list-bugcrowd       List BugCrowd programs
  --list-hackerone      List HackerOne programs
  --list-intigriti      List Intigriti programs
  --list-external       List Self-hosted programs
  --list-swags          List programs offering swag
  --list-rewards        List programs with rewards
  --list-norewards      List programs with no rewards
  --list-new            List new programs
  --list-updated        List updated programs
  -download <program>   Download a specific program
  --download-all        Download all programs
  --download-bugcrowd   Download BugCrowd programs
  --download-hackerone  Download HackerOne programs
  --download-intigriti  Download Intigriti programs
  --download-external   Download External programs
  --download-swags      Download programs offering swag
  --download-rewards    Download programs with rewards
  --download-norewards  Download programs with no rewards
  --download-new        Download new programs
  --download-updated    Download updated programs
```
