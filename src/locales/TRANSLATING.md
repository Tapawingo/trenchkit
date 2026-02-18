# Adding a New Translation

This guide explains how to translate TrenchKit into your language. No build tools or programming experience required.

## Getting started

### 1. Fork and clone the repository

1. Click **Fork** on the [TrenchKit GitHub page](https://github.com/Tapawingo/TrenchKit)
2. Clone your fork:
   ```sh
   git clone https://github.com/<your-username>/TrenchKit.git
   cd TrenchKit
   ```
3. Create a branch for your translation:
   ```sh
   git checkout -b translations/<code>
   ```
   Replace `<code>` with your [ISO 639-1 language code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes) (e.g. `fr`, `de`, `es`, `zh_CN`).

### 2. Create your translation file

Copy the template and rename it to your language code:

```sh
cp src/locales/TrenchKit_template.ts src/locales/TrenchKit_<code>.ts
```

Open the file and change the `language` attribute in the `<TS>` root element:

```xml
<!-- Before -->
<TS version="2.1" language="" sourcelanguage="en">

<!-- After (example: French) -->
<TS version="2.1" language="fr" sourcelanguage="en">
```

### 3. Translate

Open the file in any text editor. The `<source>` element is the English text; edit only the `<translation>` element:

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

### 6. Commit and push

```sh
git add src/locales/TrenchKit_<code>.ts
git commit -m "Add <language> translation"
git push origin translations/<code>
```

### 7. Open a pull request

Go to your fork on GitHub and click **Compare & pull request**. Target the `main` branch of the original repository.

## File structure

```
src/locales/
  TRANSLATING.md          # This file
  TrenchKit_template.ts   # Empty template (copy this)
  TrenchKit_en.ts         # English (plural forms only, auto-generated)
  TrenchKit_nb.ts         # Norwegian Bokmål
  TrenchKit_<code>.ts     # Your translation
```
