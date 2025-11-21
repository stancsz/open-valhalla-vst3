# Open Valhalla (VST3) — Quickstart

[![Download v0.1.0](https://img.shields.io/badge/Download-v0.1.0-brightgreen?style=for-the-badge)](https://github.com/stancsz/VST3-Open-Valhalla/releases/tag/v0.1.0)

Open Valhalla is a VST3 reverb plugin. The badge above links to the v0.1.0 release where prebuilt binaries are available.

Quick usage — get it running in your DAW
1. Click the "Download" badge above or open the release page: https://github.com/stancsz/VST3-Open-Valhalla/releases/tag/v0.1.0
2. Download and unzip the archive for your platform (e.g., `Open_Valhalla_VST3_Windows.zip` for Windows).
3. Install the plugin:
   - Windows: copy `Open Valhalla.vst3` to `C:\Program Files\Common Files\VST3` (or your DAW's VST3 scan folder).
   - macOS: copy `Open Valhalla.vst3` to `/Library/Audio/Plug-Ins/VST3` (or `~/Library/Audio/Plug-Ins/VST3`).
   - Linux: place the `.vst3` bundle in `~/.vst3` or your system VST3 path (varies by distribution/host).
4. In your DAW: rescan plugins (or relaunch the host) and insert "Open Valhalla" on a track.

Build from source
- Requirements: CMake, a supported C++ toolchain (Visual Studio / clang / gcc), and platform build tools.
- Provided scripts:
  - Windows:
```bash
build_release_windows.bat
```
  - macOS:
```bash
./build_release_mac.sh
```
  - Linux:
```bash
./build_release_linux.sh
```
These scripts create the release artifacts in the repository's `release/` folder.

If the plugin doesn't appear in your host
- Confirm the `.vst3` file is in a folder the DAW scans.
- Rescan or restart the host application.
- Check file permissions (macOS/Linux) and architecture (x64 vs arm64) compatibility.
- See `release/` for prebuilt zips.

Repository notes
- Release artifacts are stored under `release/`.
- Example Windows release: `release/Open_Valhalla_VST3_Windows.zip`
- UI screenshot: `docs/screenshot.png`

Contributing
- Fork, make changes, and open a PR.
- When changing UI assets, update the plugin screenshot (see AGENTS.md rules).

License
- Check repository for license file or contact the author for licensing details.
