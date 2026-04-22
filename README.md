# Pico SDK Template (C)

This project uses the Raspberry Pi Pico SDK with a small set of VS Code tasks
and settings to build, flash, and debug.

## Board And Chip Selection

There are two separate choices in this project:

- Board type: `pico`, `pico_w`, `pico2`, or `pico2_w`
- Debug/OpenOCD target: `rp2040` or `rp2350`

Use this mapping:

- RP2040 without wireless: `PICO_BOARD=pico`, `PICO_UPLOAD_TARGET=rp2040`
- RP2040 with wireless: `PICO_BOARD=pico_w`, `PICO_UPLOAD_TARGET=rp2040`
- RP2350 without wireless: `PICO_BOARD=pico2`, `PICO_UPLOAD_TARGET=rp2350`
- RP2350 with wireless: `PICO_BOARD=pico2_w`, `PICO_UPLOAD_TARGET=rp2350`

W/non-W changes the board definition. RP2040/RP2350 changes the chip/debug
target.

### Every Place You Need To Check

#### 1. Top-level `CMakeLists.txt`

This is the main place to switch boards and chips.

- `set(MY_PICO_BOARD ...)` selects the board definition used by the SDK.
- `set(PICO_UPLOAD_TARGET ...)` selects the OpenOCD target file used by flash
  and remote debug tasks.
- `set(PICO_JLINK_CORE ...)` selects the J-Link core: `0` or `1`

Values to use:

- `pico` for RP2040 without wireless
- `pico_w` for RP2040 with wireless
- `pico2` for RP2350 without wireless
- `pico2_w` for RP2350 with wireless
- `rp2040` for RP2040-family debug/upload target
- `rp2350` for RP2350-family debug/upload target
- `0` for J-Link core 0
- `1` for J-Link core 1

When you change either one, run a fresh configure/build so
`build/project_configuration.sh` and `build/project_configuration.env` are
regenerated with the new values.

#### 2. `.vscode/settings.json`

These VS Code settings must match the same chip family:

- `pico.uploadTarget`
- `pico.uploadTargetUppercase`
- `pico.jlinkSerialNumber`

Use:

- `rp2040` and `RP2040` for RP2040 / Pico / Pico W
- `rp2350` and `RP2350` for RP2350 / Pico 2 / Pico 2 W
- your J-Link probe serial number for `pico.jlinkSerialNumber`

These values are used by launch configurations for:

- OpenOCD target file: `target/${config:pico.uploadTarget}.cfg`
- SVD path:
  `${config:cmake.configureEnvironment.PICO_SDK_PATH}/src/${config:pico.uploadTarget}/hardware_regs/${config:pico.uploadTargetUppercase}.svd`
- Cortex-Debug device selection

#### 3. `.vscode/launch.json`

Most launch configs already read `${config:pico.uploadTarget}` or
`${config:pico.uploadTargetUppercase}`, so they follow `.vscode/settings.json`.

The `Pico Debug (Cortex-Debug Remote OpenOCD)` launcher now uses a simple
`monitor reset halt` pre-launch command instead of target-specific
`arp_reset` commands. This keeps the remote launcher compatible with both the
RP2040 and RP2350 OpenOCD target scripts.

The one place that still needs separate attention is the `J-Link` launch
configuration:

- `device` is hard-coded to `RP2040_M0_0`
- `serialNumber` now reads `${config:pico.jlinkSerialNumber}`

That means:

- For RP2040, the current J-Link entry is aligned
- For RP2350, you must update that J-Link `device` setting before using that
  launcher

If you do not use J-Link, you can ignore this section.

#### 4. `.vscode/tasks.json`

The task file does not define the board directly, but several tasks consume the
generated `PICO_UPLOAD_TARGET` from `build/project_configuration.sh`.

These tasks therefore depend on the value selected in `CMakeLists.txt`:

- `Flash(SSH)`
- `Probe(Upload)`
- `Debug Serv`
- `SSH Upload`

There is also a J-Link task:

- `Flash(J-Link)` now runs the helper script `.vscode/flash_jlink.sh`
- that script reads `PICO_UPLOAD_TARGET` and `PICO_JLINK_CORE`
- that task passes `${config:pico.jlinkSerialNumber}` into the script

That means:

- RP2040 core 0 -> `RP2040_M0_0`
- RP2040 core 1 -> `RP2040_M0_1`
- RP2350 core 0 -> `RP2350_M33_0`
- RP2350 core 1 -> `RP2350_M33_1`

J-Link helper script location:

- `.vscode/flash_jlink.sh`

That script:

- sources `build/project_configuration.sh`
- creates a temporary J-Link commander script
- maps chip family and selected core to the correct J-Link `-device`
- receives the J-Link serial number from `.vscode/settings.json`
- runs `/usr/local/bin/JLinkExe`

The main compile task currently does a fresh debug configure before building:

- `--fresh`
- `-DCMAKE_BUILD_TYPE=Debug`
- `-DPICO_DEOPTIMIZED_DEBUG=1`

This makes debug builds more deterministic, at the cost of a slower configure
step than a normal incremental build.

#### 5. Source Code That Depends On Board Type

The current application uses:

- `src/main.c`
- `lib/digital_io/digital_io.h`
- `lib/digital_io/digital_io.c`

`src/main.c` uses:

- `PICO_DEFAULT_LED_PIN`

`digital_io` checks for CYW43 support through Pico SDK defines and only enables
the special onboard LED path on W boards. The relevant symbol is:

- `PICO_W_LED_PIN`

Implications:

- On `pico_w` and `pico2_w`, `PICO_W_LED_PIN` maps to the CYW43 LED
- On non-W boards, `PICO_W_LED_PIN` becomes invalid and must not be used as a
  real GPIO
- `PICO_DEFAULT_LED_PIN` in `src/main.c` still resolves according to the board
  selected in the Pico SDK

If your application code uses either the board default LED or the Pico W
special LED handling, the board selection must match the actual hardware.

#### 6. Generated Build Files

These are generated from `CMakeLists.txt` during configure:

- `build/project_configuration.sh`
- `build/project_configuration.env`

They are not the place to edit settings manually. They are outputs that tasks
and debug configurations read after CMake writes:

- `PICO_BOARD`
- `PICO_UPLOAD_TARGET`
- `PICO_JLINK_CORE`

## Board-Related File List

These are the files involved when switching between RP2040/RP2350 and W/non-W:

- `CMakeLists.txt`
- `.vscode/settings.json`
- `.vscode/tasks.json`
- `.vscode/launch.json`
- `.vscode/flash_jlink.sh`
- `src/main.c`
- `lib/digital_io/digital_io.h`
- `lib/digital_io/digital_io.c`
- `build/project_configuration.sh` generated
- `build/project_configuration.env` generated

## Quick Switch Checklist

For RP2040 Pico:

- `CMakeLists.txt` -> `MY_PICO_BOARD=pico`
- `CMakeLists.txt` -> `PICO_UPLOAD_TARGET=rp2040`
- `CMakeLists.txt` -> `PICO_JLINK_CORE=0` or `1`
- `.vscode/settings.json` -> `pico.uploadTarget=rp2040`
- `.vscode/settings.json` -> `pico.uploadTargetUppercase=RP2040`
- `src/main.c` -> `PICO_DEFAULT_LED_PIN` now resolves for `pico`

For RP2040 Pico W:

- `CMakeLists.txt` -> `MY_PICO_BOARD=pico_w`
- `CMakeLists.txt` -> `PICO_UPLOAD_TARGET=rp2040`
- `CMakeLists.txt` -> `PICO_JLINK_CORE=0` or `1`
- `.vscode/settings.json` -> `pico.uploadTarget=rp2040`
- `.vscode/settings.json` -> `pico.uploadTargetUppercase=RP2040`
- `src/main.c` -> `PICO_DEFAULT_LED_PIN` now resolves for `pico_w`

For RP2350 Pico 2:

- `CMakeLists.txt` -> `MY_PICO_BOARD=pico2`
- `CMakeLists.txt` -> `PICO_UPLOAD_TARGET=rp2350`
- `CMakeLists.txt` -> `PICO_JLINK_CORE=0` or `1`
- `.vscode/settings.json` -> `pico.uploadTarget=rp2350`
- `.vscode/settings.json` -> `pico.uploadTargetUppercase=RP2350`
- `src/main.c` -> `PICO_DEFAULT_LED_PIN` now resolves for `pico2`
- `.vscode/launch.json` J-Link entry must be reviewed before J-Link debugging
- `.vscode/tasks.json` `Flash(J-Link)` task must be reviewed before J-Link
  flashing

For RP2350 Pico 2 W:

- `CMakeLists.txt` -> `MY_PICO_BOARD=pico2_w`
- `CMakeLists.txt` -> `PICO_UPLOAD_TARGET=rp2350`
- `CMakeLists.txt` -> `PICO_JLINK_CORE=0` or `1`
- `.vscode/settings.json` -> `pico.uploadTarget=rp2350`
- `.vscode/settings.json` -> `pico.uploadTargetUppercase=RP2350`
- `src/main.c` -> `PICO_DEFAULT_LED_PIN` now resolves for `pico2_w`
- `.vscode/launch.json` J-Link entry must be reviewed before J-Link debugging
- `.vscode/tasks.json` `Flash(J-Link)` task must be reviewed before J-Link
  flashing

## After Changing Board Type

1. Update `CMakeLists.txt`
2. Update `.vscode/settings.json`
3. Reconfigure/build the project
4. Then run flash/debug tasks

## Pico SDK and Toolchain Paths

The expected SDK and toolchain locations are set in `.vscode/settings.json`:

- `cmake.configureEnvironment.PICO_SDK_PATH` points at the Pico SDK root
  (for example `${HOME}/.pico-sdk/sdk/2.2.0`).
- `cmake.configureEnvironment.PICO_TOOLCHAIN_PATH` points at the GCC ARM
  toolchain root (for example `${HOME}/.pico-sdk/toolchain/14_2_Rel1`).
- `cmake.configureEnvironment.PATH` should include
  `${PICO_TOOLCHAIN_PATH}/bin` so `arm-none-eabi-gcc` is on PATH.

If you use `cmake.buildEnvironment`, keep these values in sync there too.

## Firmware Output Name

The ELF name is controlled by the target name in `src/CMakeLists.txt`:

- `set(FIRMWARE_NAME ...)` defines the executable name.
- `add_executable(${FIRMWARE_NAME} ...)` uses that name for the output files.

Tasks reference the ELF name via `PICO_FIRMWARE_NAME` in the top-level
`CMakeLists.txt`. Keep these two values the same:

- `src/CMakeLists.txt` -> `FIRMWARE_NAME`
- `CMakeLists.txt` -> `PICO_FIRMWARE_NAME`

If you change one, change the other and rebuild so the new ELF exists in
`build/`.

## Serial Output Helpers

The `serial_print` helper wraps Pico SDK stdio and currently lives in:

- `lib/serial_print/serial_print.h`
- `lib/serial_print/serial_print.c`

Current behavior:

- `serial_init()` calls `stdio_init_all()` once and is safe to call more than
  once
- string helpers ignore `NULL` inputs
- output uses the stdio backends enabled for the firmware target

In the current firmware target in `src/CMakeLists.txt`, both USB and UART stdio
are enabled:

- `pico_enable_stdio_usb(... 1)`
- `pico_enable_stdio_uart(... 1)`

## UF2 Output Location

UF2 output is controlled by `PICO_UF2_OUTPUT_DIR` in the top-level
`CMakeLists.txt`. It defaults to the build root (`build/`), so the UF2 ends up
alongside the ELF.

## Probe Serial Number

The probe serial is set in the top-level `CMakeLists.txt`:

- `CMSIS_DAP_SERIAL_NUMBER` is written into `build/project_configuration.sh`
  during CMake configure.
- Tasks source `build/project_configuration.sh` to read
  `CMSIS_DAP_SERIAL_NUMBER` and pass it to OpenOCD.

To select a different probe:

1. Update `CMSIS_DAP_SERIAL_NUMBER` in the top-level `CMakeLists.txt`.
2. Reconfigure (or run a build task that reruns CMake) to regenerate
   `build/project_configuration.sh`.

## VS Code Tasks

Common tasks in `.vscode/tasks.json`:

- `Compile Project`: Configure and build in `build/`.
- `Clean Build`: Delete `build/`, then configure and build.
- `Probe(Upload)`: Uses OpenOCD locally with the probe serial from
  `build/project_configuration.sh` and flashes `build/${PICO_FIRMWARE_NAME}.elf`.
- `Flash(USB)`: Uses `picotool load` to flash
  `build/${PICO_FIRMWARE_NAME}.elf`.
- `Upload .elf(SSH)`: Copies the ELF to the remote host.
- `Remote Upload`: Runs OpenOCD on the remote host to flash the ELF.
- `Delete .elf(SSH)`: Removes the ELF on the remote host.
- `Debug Server`: Starts OpenOCD on the remote host without flashing.
- `Kill Serv`: Sends `shutdown` to the remote OpenOCD telnet port (4444),
  falling back to `pkill` if the telnet command fails.

Remote settings live in the top-level `CMakeLists.txt` and are written into
`build/project_configuration.sh` during configure:

- `PICO_REMOTE_HOST`
- `PICO_REMOTE_USER`
- `PICO_REMOTE_PATH`
- `PICO_FIRMWARE_NAME`
- `PICO_SDK_VERSION`
- `PICO_UPLOAD_TARGET` (OpenOCD target file like `rp2040` or `rp2350`)

These are used by the upload and remote OpenOCD tasks.
Ensure the remote host allows SSH connections from your machine and that the
`picotemp` folder exists on the remote host at the configured
`PICO_REMOTE_PATH`.

Note on `Kill Serv` output:

- The `��������Open On-Chip Debugger` text is OpenOCD's telnet banner echoed by
  `nc`; the garbled prefix is an encoding/terminal mismatch and is harmless.

## Debug Configurations

Launch configs live in `.vscode/launch.json`. The key differences:

- `Pico Debug (Cortex-Debug)` starts its own OpenOCD session.
- `Pico Debug (Cortex-Debug with external OpenOCD)` expects you to start
  OpenOCD separately on your computer and then connects to it.
- `Pico Debug Remote (Cortex-Debug Remote OpenOCD)` uses an external OpenOCD
  server, connects to `${config:pico.uploadHostName}:3333`, and resets both
  cores via `${config:pico.uploadTarget}` before halting.

Launch configs use:

- `${command:cmake.launchTargetPath}` for the ELF path.
- `${config:pico.uploadTarget}` for the OpenOCD target file.

Build before debug:

- CMake Tools builds before launch when `"cmake.buildBeforeRun": true` in
  `.vscode/settings.json`.
- The `Pico Debug (C++ Debugger)` config also runs its `preLaunchTask`
  (currently `Probe(Upload)` in `.vscode/launch.json`), which depends on
  `Compile Project`.

## VS Code Settings

Settings in `.vscode/settings.json`:

- `pico.uploadTarget`: OpenOCD target file name (e.g. `rp2040` or `rp2350`).
  Required by `Cortex-Debug` launch configs to load the matching target file
  and issue per-core reset commands.
- `pico.uploadHostName`: Remote OpenOCD hostname (e.g. `mypi5.local`).
  Required by `Pico Debug Remote` to connect GDB to the remote OpenOCD server.

Project parameters like remote host/user/path, firmware name, and upload target
are defined in `CMakeLists.txt` and flow into tasks via
`build/project_configuration.sh`.

Note on `${command:cmake.launchTargetPath}`:

- This is provided by the Raspberry Pi Pico VS Code extension at runtime.
- It is not defined in project files. Tasks now use the explicit path
  `build/${PICO_FIRMWARE_NAME}.elf` instead.
