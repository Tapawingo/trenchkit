# Adding a New Translation

This guide explains how to add a new language translation to TrenchKit.

## Prerequisites

- Qt 6.2+ with the LinguistTools component installed
- CMake 3.24+
- A working build environment (see project README for build instructions)

## Steps

### 1. Register the language in CMake

Open `CMakeLists.txt` (project root) and add your language code to `I18N_TRANSLATED_LANGUAGES`:

```cmake
# Before (Norwegian only):
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES nb)

# After (Norwegian + French):
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES nb fr)
```

Use [ISO 639-1 language codes](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes) (e.g. `de`, `fr`, `es`, `ja`, `zh`).

### 2. Generate the .ts file

Reconfigure and run the `update_translations` target:

```sh
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug --target update_translations
```

This creates `src/i18n/TrenchKit_<code>.ts` (e.g. `TrenchKit_fr.ts`) with all source strings ready for translation.

### 3. Translate

Open the `.ts` file in **Qt Linguist** (recommended) or edit the XML directly.

**Using Qt Linguist:**

```sh
# Qt Linguist is included with your Qt installation
linguist src/i18n/TrenchKit_fr.ts
```

Qt Linguist provides context, source locations, and validation. Mark each entry as "finished" (green checkmark) when done.

**Editing XML directly:**

Each entry looks like this:

```xml
<message>
    <source>Check for Updates</source>
    <translation type="unfinished"></translation>
</message>
```

Fill in the translation and remove `type="unfinished"`:

```xml
<message>
    <source>Check for Updates</source>
    <translation>Rechercher des mises à jour</translation>
</message>
```

### 4. Translation guidelines

- **Keep technical terms** commonly used in English in your language's gaming/modding community (e.g. "mod", "pak", "API key", "SSO")
- **Keep brand names** as-is: TrenchKit, Foxhole, Nexus Mods, itch.io, GitHub
- **Preserve format specifiers** like `%1`, `%2` in the correct positions
- **Preserve HTML tags** (`<b>`, `<br>`, `<a href="...">`, etc.)
- **Keep file extensions and paths** untranslated (`.pak`, `.zip`, `.tkprofile`, `AppData/...`)
- **Don't translate** `QStringLiteral()` strings, log messages, object names, or settings keys (these are not in the .ts file)

### 5. Build and verify

```sh
cmake --build build/debug
```

The build output should show your translation being compiled:

```
Generating src/TrenchKit_fr.qm
    Generated 412 translation(s) (412 finished and 0 unfinished)
```

The `.qm` files are automatically embedded as Qt resources at `:/i18n/` and loaded by `TranslationManager` at runtime.

### 6. Test

1. Launch TrenchKit
2. Open Settings
3. Select your language from the Language dropdown
4. Click Save
5. Verify the UI updates to your language

## External translation testing (no build tools required)

If you want to test a translation without building the project:

### 1. Get the template

Copy `TrenchKit_en.ts` from the `src/i18n/` directory in the repository.

### 2. Rename and set the language

Rename the file to `TrenchKit_<code>.ts` (e.g. `TrenchKit_fr.ts`) and change the `language` attribute in the `<TS>` root element:

```xml
<!-- Before -->
<TS version="2.1" language="en" sourcelanguage="en">

<!-- After -->
<TS version="2.1" language="fr" sourcelanguage="en">
```

### 3. Translate

Edit the XML in any text editor. Fill in `<translation>` elements and remove `type="unfinished"` for completed entries. See "Translation guidelines" above.

### 4. Place the file

Create a `locales/` folder next to the TrenchKit executable and place your `.ts` file there:

```
TrenchKit.exe
locales/
    TrenchKit_fr.ts
```

### 5. Select the language

Restart TrenchKit, open Settings, and select your language from the dropdown. External `.ts` files are loaded automatically and take priority over the built-in translations.

Entries marked `type="unfinished"` with empty translation text are skipped (the English source text is shown instead), so you can translate incrementally.

## File structure

```
src/i18n/
  TRANSLATING.md          # This file
  TrenchKit_en.ts         # English (plural-only forms, auto-generated)
  TrenchKit_nb.ts         # Norwegian Bokmål
  TrenchKit_<code>.ts     # Your translation
```

## How it works

- Source strings are wrapped with `tr()` in C++ code
- `lupdate` extracts these into `.ts` files (XML)
- `lrelease` compiles `.ts` into binary `.qm` files
- `.qm` files are embedded as Qt resources via `qt_add_translations()`
- `TranslationManager` loads the appropriate `.qm` at startup and on language change
- External `.ts` files in `locales/` are parsed at runtime by `TsTranslator` and take priority over embedded `.qm` files
- Available languages are discovered automatically from both embedded resources and external files
- Qt sends `QEvent::LanguageChange` to all widgets, which call `retranslateUi()` to update their text

## Updating an existing translation

When new strings are added to the codebase, run:

```sh
cmake --build build/debug --target update_translations
```

This updates all `.ts` files, adding new entries as `type="unfinished"` while preserving existing translations. Search for `unfinished` in your `.ts` file to find strings that need translating.
