# Launching Supermodel UWP from EmulationStation

This branch adds direct ROM activation to the Xbox/UWP build. The package
manifest already registers the `.zip` file association and the `supermodel:`
URI scheme; the application now consumes those activation arguments instead of
always opening the file picker.

## Supported launch forms

Use one of these URI forms (values must be URL encoded):

```text
supermodel:?rom=D%3A%5CRoms%5CModel3%5Cdaytona2.zip
supermodel:?path=D%3A%5CRoms%5CModel3%5Cdaytona2.zip
supermodel:?file=D%3A%5CRoms%5CModel3%5Cdaytona2.zip
```

The `cmd` form accepts a ROM followed by normal Supermodel options:

```text
supermodel:?cmd=%22D%3A%5CRoms%5CModel3%5Cdaytona2.zip%22%20-fullscreen
```

For frontends that must be relaunched after the emulator closes, append their
registered URI scheme:

```text
supermodel:?rom=D%3A%5CRoms%5CModel3%5Cdaytona2.zip&launchOnExit=myfrontend
```

A full URI is also accepted as the value of `launchOnExit`. In XML configuration
files write the separator as `&amp;`.

## EmulationStation / CoinOPS setup

Configure the Model 3 emulator entry to launch the registered UWP protocol and
substitute the selected ROM for the `rom` value:

```text
supermodel:?rom=<URL-ENCODED-ROM-PATH>
```

The exact placeholder and URI-launch helper depend on the EmulationStation or
CoinOPS build. The important contract is that the final URI received by Windows
matches one of the forms above. If the frontend already uses the RetroArch UWP
convention, use the `cmd` form and optionally `launchOnExit`.

## Other activation paths

Opening a `.zip` file through Windows also launches it directly. Starting
Supermodel without an activation payload keeps the existing behavior and opens
the ROM picker.

## Notes

- Only the first ROM is loaded; this matches Supermodel's command-line behavior.
- The package needs filesystem permission for the ROM location. The manifest
  already declares `broadFileSystemAccess` and `removableStorage`.
- Xbox deployment still requires an x64 MSIX/APPX signed for the console's Dev
  Mode environment.
