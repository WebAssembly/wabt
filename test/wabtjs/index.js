"use strict"
var fs = require("fs");
var test = require("tape");

// This test case exists to catch the most obvious issues before pushing the binary back to GitHub.
// It's not intended to be a full test suite, but feel free to extend it / send a PR if necessary!

require("../../emscripten/libwabt")().then(wabt => {

  var mod;
  test("loading a binary module", function (test) {
    var buffer = new Uint8Array(
      fs.readFileSync(__dirname + "/assembly/module.wasm")
    );
    test.doesNotThrow(function () {
      mod = wabt.readWasm(buffer, { readDebugNames: true });
    });
    test.ok(mod && typeof mod.toBinary === "function", "should return a module");
    test.end();
  });

  test("loading a binary module (with features)", function (test) {
    var buffer = new Uint8Array(
      fs.readFileSync(__dirname + "/assembly/module-features.wasm")
    );
    test.doesNotThrow(function () {
      mod = wabt.readWasm(buffer, {
        readDebugNames: true,
      });
    });
    test.ok(mod && typeof mod.toBinary === "function", "should return a module");
    test.end();
  });

  test("modifying a module", function (test) {
    test.doesNotThrow(function () {
      mod.generateNames();
      mod.applyNames();
    });
    test.end();
  });

  test("emitting a module", function (test) {
    var text, binaryRes;
    test.doesNotThrow(function () {
      text = mod.toText({ foldExprs: true, inlineExport: false });
      binaryRes = mod.toBinary({ write_debug_names: true });
    });
    test.ok(
      typeof text === "string" && text.length,
      "should return a string from calling Module#toText"
    );
    test.ok(
      binaryRes &&
      binaryRes.buffer &&
      binaryRes.buffer.length &&
      typeof binaryRes.log === "string",
      "should return a binary result from calling Module#toBinary"
    );
    test.end();
  });

  test("destroying a module", function (test) {
    test.doesNotThrow(function () {
      mod.destroy();
    }, "should not throw when calling Module#destroy");
    test.end();
  });

  test("loading a text (wat) module", function (test) {
    var str = fs.readFileSync(__dirname + "/assembly/module.wat").toString();
    var mod;
    test.doesNotThrow(function () {
      mod = wabt.parseWat("module.wat", str);
    });
    test.ok(mod && typeof mod.toBinary === "function", "should return a module");
    test.doesNotThrow(function () {
      mod.destroy();
    }, "should not throw when calling Module#destroy");
    test.end();
  });

  test("loading a text (wat) module with features", function (test) {
    var str = fs.readFileSync(__dirname + "/assembly/module-features.wat").toString();
    var mod;
    test.doesNotThrow(function () {
      mod = wabt.parseWat("module-features.wat", str);
    });
    test.ok(mod && typeof mod.toBinary === "function", "should return a module");
    test.doesNotThrow(function () {
      mod.destroy();
    }, "should not throw when calling Module#destroy");
    test.end();
  });

});
