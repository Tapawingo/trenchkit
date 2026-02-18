# Adding a New Translation

This guide explains how to add a new language translation to TrenchKit. No build tools are required.

## Steps

### 1. Get the template

Copy `TrenchKit_template.ts` from the `src/locales/` directory in this repository. It contains all translatable strings with empty translations ready to fill in.

### 2. Rename and set the language

Rename the file to `TrenchKit_<code>.ts` (e.g. `TrenchKit_fr.ts`) using an [ISO 639-1 language code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes), and change the `language` attribute in the `<TS>` root element:

```xml
<!-- Before -->
<TS version="2.1" language="" sourcelanguage="en">

<!-- After -->
<TS version="2.1" language="fr" sourcelanguage="en">
```

### 3. Translate

Open the file in any text editor. Replace existing translations with your own. The `<source>` element is the English text; edit only the `<translation>` element:

```xml
<message>
    <source>Check for Updates</source>
    <translation>Rechercher des mises à jour</translation>
</message>
```

To mark an entry as not yet translated, set `type="unfinished"` with empty text. These are skipped at runtime and the English source text is shown instead, so you can translate incrementally:

```xml
<translation type="unfinished"></translation>
```

### 4. Translation guidelines

- **Keep technical terms** commonly used in English in your language's gaming/modding community (e.g. "mod", "pak", "API key", "SSO")
- **Keep brand names** as-is: TrenchKit, Foxhole, Nexus Mods, itch.io, GitHub
- **Preserve format specifiers** like `%1`, `%2` in the correct positions
- **Preserve HTML tags** (`<b>`, `<br>`, `<a href="...">`, etc.)
- **Keep file extensions and paths** untranslated (`.pak`, `.zip`, `.tkprofile`, `AppData/...`)

### 5. Test your translation

Create a `locales/` folder next to the TrenchKit executable and place your `.ts` file there:

```
TrenchKit.exe
locales/
    TrenchKit_fr.ts
```

Restart TrenchKit, open Settings, and select your language from the dropdown.

### 6. Submit

Once you're happy with the translation, open a pull request adding your `.ts` file to `src/locales/`.

## File structure

```
src/locales/
  TRANSLATING.md          # This file
  TrenchKit_template.ts   # Empty template (copy this)
  TrenchKit_en.ts         # English (plural forms only, auto-generated)
  TrenchKit_nb.ts         # Norwegian Bokmål
  TrenchKit_<code>.ts     # Your translation
```
