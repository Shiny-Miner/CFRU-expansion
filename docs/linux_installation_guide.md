# Building CFRU-expansion on Linux

This guide covers Ubuntu, Debian, and Fedora with Bash on **x86_64** computers. Follow the section for your distribution, then complete the shared steps. Installation commands use `sudo`; build the project as a regular user.

The dependencies were identified from `scripts/make.py`, `scripts/build.py`, `scripts/insert.py`, `scripts/trainer_party.py`, and `scripts/follower_mon_sprites.py`. The full setup has not been tested on clean installations of all three distributions.

## 1. Requirements

| Dependency | Purpose |
| --- | --- |
| Python 3 (use 3.10 or later) | Run the build and data generation scripts. |
| Pillow (`PIL`) | Convert follower Pokémon sprites; required for the normal build. |
| GCC for `arm-none-eabi` and Binutils | Compile C, assemble Assembly code, link objects, and extract symbols and binaries. |
| Newlib for ARM | Provide C headers such as `string.h`. |
| `grit` | Convert PNG/BMP graphics. |
| `wav2agb` | Convert WAV samples. |
| `mid2agb` | Convert MIDI music. |
| Git, native C/C++ compilers, Make, and Autotools | Download and build the converters for Linux. |
| FreeImage development files | Build and run `grit`. |

The entry point is `python3 scripts/make.py`, run from the project root. The `.bat` files are Windows shortcuts. Make, installed below, is used to build the supporting tools.

### Compiler compatibility

The inspected local environment uses **devkitARM r47, GCC 7.1.0**. The commands below install the ARM GCC version available in your distribution, which may differ. Installing the dependencies does not guarantee compatibility with every recent GCC version: differences in language rules, symbols, and code generation may require adjustments or the devkitARM version used by the project.

If you already have devkitARM, you can use its `arm-none-eabi-*` executables and headers instead of the ARM GCC/Binutils/Newlib packages listed below. Place that installation's `bin` directory before `/usr/bin` in your `PATH` and verify it with `arm-none-eabi-gcc --version`. See the [official devkitPro toolchain distribution](https://github.com/devkitPro/buildscripts) for toolchain information.

## 2. Install distribution packages

### Ubuntu

Enable the Universe repository, which contains [ARM GCC](https://packages.ubuntu.com/noble/gcc-arm-none-eabi):

```bash
sudo apt update
sudo apt install software-properties-common
sudo add-apt-repository -y universe
sudo apt update
sudo apt install --no-install-recommends \
  git ca-certificates build-essential autoconf automake libtool pkg-config \
  libfreeimage-dev python3 python3-pil \
  gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
```

### Debian

```bash
sudo apt update
sudo apt install --no-install-recommends \
  git ca-certificates build-essential autoconf automake libtool pkg-config \
  libfreeimage-dev python3 python3-pil \
  gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
```

On Ubuntu and Debian, `--no-install-recommends` avoids installing the recommended ARM C++ libraries, which are not required for this project's C sources.

### Fedora

```bash
sudo dnf install \
  git ca-certificates gcc gcc-c++ make autoconf automake libtool pkgconf-pkg-config \
  freeimage-devel python3 python3-pillow \
  arm-none-eabi-gcc-cs arm-none-eabi-binutils-cs arm-none-eabi-newlib
```

The compiler and library package names differ on Fedora: [arm-none-eabi-gcc-cs](https://packages.fedoraproject.org/pkgs/arm-none-eabi-gcc-cs/arm-none-eabi-gcc-cs/) and [arm-none-eabi-newlib](https://packages.fedoraproject.org/pkgs/arm-none-eabi-newlib/arm-none-eabi-newlib/).

## 3. Set up PATH

The tools built in the following steps will be installed in `~/.local/bin`:

```bash
mkdir -p "$HOME/.local/bin" "$HOME/.local/src/cfru-tools"
export PATH="$HOME/.local/bin:$PATH"
```

To keep this setting in new Bash terminals, append this line to `~/.bashrc` if it is not already present:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

## 4. Install grit

[devkitPro's grit](https://github.com/devkitPro/grit) uses Autotools and FreeImage. Run:

```bash
cd "$HOME/.local/src/cfru-tools"
git clone https://github.com/devkitPro/grit.git
cd grit
autoreconf -fi
./configure --prefix="$HOME/.local"
make -j2
make install
```

Run each command only if the previous one completes without errors. If you have already cloned this directory, enter it instead of repeating `git clone`.

## 5. Install wav2agb

Use [ipatix's native implementation](https://github.com/ipatix/wav2agb), which accepts the `input.wav output.s` argument order used by this project's script:

```bash
cd "$HOME/.local/src/cfru-tools"
git clone https://github.com/ipatix/wav2agb.git
cd wav2agb
make -j2
install -m 755 wav2agb "$HOME/.local/bin/wav2agb"
```

This tool is built with Linux's `g++`. The `.exe` executables in `deps/` are not used by the native Linux build path.

## 6. Prepare the project and mid2agb

Return to your checkout directory. Replace the path below with the actual path:

```bash
cd /path/to/CFRU-expansion
chmod +x deps/mid2agb
```

The repository already includes `deps/mid2agb`, an **x86_64** Linux executable. `scripts/build.py` gives this file priority; it only searches for `mid2agb` in `PATH` when the local file does not exist. You do not need another MIDI converter on a compatible machine.

On ARM64 Linux, this binary cannot run natively. You will need to replace the local file with a converter build compatible with your architecture and the options used by the project. This guide does not cover that adaptation.

### Input ROM

Place your base ROM, compatible with Pokémon FireRed US 1.0 and prepared for this fork, in the project root as **`BPRE0.gba`**. The project injects code and data into an existing ROM; installing the compilers does not replace this input.

Check the settings in `scripts/make.py` before building:

```python
ROM_NAME = "BPRE0.gba"
OFFSET_TO_PUT = 0x1000000
SEARCH_FREE_SPACE = False
```

The insertion address is fixed by default. The base ROM must have reserved space compatible with this fork's settings, including follower sprites at `0x900000`. Also consult the base preparation instructions linked in the [project README](../README.md).

## 7. Check dependencies

From the project root:

```bash
python3 --version
python3 -c 'from PIL import Image; print("Pillow available:", Image.__version__)'
arm-none-eabi-gcc --version

for tool in arm-none-eabi-gcc arm-none-eabi-as arm-none-eabi-ld \
  arm-none-eabi-objcopy arm-none-eabi-objdump arm-none-eabi-nm grit wav2agb
do
  command -v "$tool" || echo "MISSING: $tool"
done

test -x deps/mid2agb && echo "Local mid2agb is executable"
test -f BPRE0.gba && echo "Base ROM found"
```

Resolve any missing dependencies before continuing. Checking that the commands exist does not replace a full build.

## 8. Build

Always run from the project root:

```bash
python3 scripts/make.py
```

The script updates `linker.ld` and the insertion settings in `scripts/insert.py`, compiles the sources, and inserts the result into **`test.gba`**. Any previous output with that name will be overwritten. The process also generates `build/output.bin`, `build/linked.o`, and `offsets.ini`.

To build only the code and assets, without inserting them into the ROM:

```bash
python3 scripts/build.py
```

This command uses the current `linker.ld` configuration; it does not perform the adjustments made by `make.py`.

After switching compilers or changing headers, you may need to rebuild the objects. The following command removes `test.gba`, `offsets.ini`, and code objects while preserving image and audio objects:

```bash
python3 scripts/clean.py build
python3 scripts/make.py
```

## 9. Troubleshooting

| Message or symptom | What to check |
| --- | --- |
| `No module named 'PIL'` | Install `python3-pil` on Ubuntu/Debian or `python3-pillow` on Fedora. Python running in a virtual environment may not see system packages. |
| `arm-none-eabi-gcc` or assembler not found | Check the toolchain installation and `PATH`. Use `command -v arm-none-eabi-gcc` to identify the installation being used. |
| `string.h: No such file or directory` | Check that Newlib for ARM is installed for the selected compiler. |
| `grit` or `wav2agb` not found | Check that `~/.local/bin` is in `PATH` and that the tool was built and installed successfully. |
| Missing `FreeImage.h` or `libfreeimage` | Install `libfreeimage-dev` or `freeimage-devel` before running grit's `configure`. |
| `Permission denied` for `deps/mid2agb` | Run `chmod +x deps/mid2agb` and check that the project is on a partition that allows executing files. |
| `Exec format error` for `deps/mid2agb` | Check the architecture with `uname -m`; the included binary is x86_64. |
| Missing shared library when starting a tool | Use `ldd deps/mid2agb` or `ldd "$HOME/.local/bin/grit"` to identify the missing library. |
| `Could not find source rom` | Check the filename `BPRE0.gba` and run the script from the project root. Linux filenames are case-sensitive. |
| C errors, duplicate symbols, or relocation errors with recent GCC versions | Check the compiler version and its compatibility with the legacy code. The inspected local environment uses devkitARM r47; do not automatically treat these errors as missing dependencies. |

Emulators, graphics editors, and the Python `unicorn` package used by the evolution tests are not required for the normal build described here.
