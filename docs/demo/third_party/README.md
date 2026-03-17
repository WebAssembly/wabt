# WABT Demo Third-Party Dependencies

This directory contains the build configuration for third-party dependencies
used by the WABT web demos (located in `docs/demo`).

Currently, this is used to generate a single ES Module bundle for **CodeMirror
6**, which is required because CodeMirror 6 consists of many highly-scoped NPM
packages that are difficult to load individually via CDNs in a browser without
instance-duplication issues.

## Building the Third-Party Bundle

The demo applications (`wasm2wat/index.html` and `wat2wasm/index.html`) expect
to find a compiled bundle at `docs/demo/third_party.bundle.js`.

To regenerate this bundle:

1. Ensure you have [Node.js and npm](https://nodejs.org/) installed.
2. From this directory (`docs/demo/third_party`), execute:

```bash
npm install
npm run build
```

The `build` script uses `esbuild` to analyze `index.js`, tree-shake the used
modules (like `EditorView`, `wast` syntax highlighting, and `javascript`
support), and output the final ES Module bundle as `../third_party.bundle.js`.

## Adding or Updating Dependencies

1. If you need to expose new CodeMirror features to the demo apps, add the
   relevant `export` statement to `index.js`.
2. To update versions of CodeMirror, run `npm update` or manually adjust
   `package.json` and run `npm install`.
3. Commit both the updated `package.json`/`package-lock.json` and the built
   `../third_party.bundle.js`.
