# Packaging

## Windows

```bat
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.10.1\msvc2022_64
cmake --build build --config Release
cmake --install build --prefix dist
iscc packaging\installer.iss
```

The installer drops out at `packaging\Output\MAPS0DesktopClient-Setup-1.0.0.exe`.
Ship `vc_redist.x64.exe` separately or have users install it once.

## macOS

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=$HOME/Qt/6.10.1/macos \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build
cmake --install build --prefix dist
cd dist && hdiutil create -volname "MAPS0 Desktop Client" \
    -srcfolder "MAPS0 Desktop Client.app" -ov -format UDZO MAPS0DesktopClient.dmg
```

For Gatekeeper-clean distribution add codesign + notarytool steps (see chat).

## Linux (AppImage)

Requires `linuxdeploy` and the `linuxdeploy-plugin-qt` binaries on `PATH`.
Build on the *oldest* glibc you want to support (Ubuntu 20.04 is a safe baseline).

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix AppDir/usr

linuxdeploy-x86_64.AppImage --appdir AppDir \
    -d packaging/maps0.desktop \
    -i packaging/maps0.png \
    --plugin qt --output appimage
```

You'll need a 256x256 `packaging/maps0.png` icon — placeholder for now.
