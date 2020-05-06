# wast2json

`wast2json` converts a `.wast` file to a `.json` file, and a collection of
associated `.wat` file and `.wasm` files.

## Example

```sh
# parse spec-test.wast, and write files to spec-test.json. Modules are written
# to spec-test.0.wasm, spec-test.1.wasm, etc.
$ bin/wast2json spec-test.wast -o spec-test.json
```

## Wast

The wast format is described in the [spec
interpreter](https://github.com/WebAssembly/spec/tree/master/interpreter#scripts).
It is an extension of the `.wat` text format, with additional commands for
running scripts. The syntax is repeated here:

```
script: <cmd>*

cmd:
  <module>                                   ;; define, validate, and initialize module
  ( register <string> <name>? )              ;; register module for imports
module with given failure string
  <action>                                   ;; perform action and print results
  <assertion>                                ;; assert result of an action
  <meta>                                     ;; meta command

module:
  ...
  ( module <name>? binary <string>* )        ;; module in binary format (may be malformed)
  ( module <name>? quote <string>* )         ;; module quoted in text (may be malformed)

action:
  ( invoke <name>? <string> <expr>* )        ;; invoke function export
  ( get <name>? <string> )                   ;; get global export

assertion:
  ( assert_return <action> <result>* )       ;; assert action has expected results
  ( assert_trap <action> <failure> )         ;; assert action traps with given failure string
  ( assert_exhaustion <action> <failure> )   ;; assert action exhausts system resources
  ( assert_malformed <module> <failure> )    ;; assert module cannot be decoded with given failure string
  ( assert_invalid <module> <failure> )      ;; assert module is invalid with given failure string
  ( assert_unlinkable <module> <failure> )   ;; assert module fails to link
  ( assert_trap <module> <failure> )         ;; assert module traps on instantiation

result:
  ( <val_type>.const <numpat> )

numpat:
  <value>                                    ;; literal result
  nan:canonical                              ;; NaN in canonical form
  nan:arithmetic                             ;; NaN with 1 in MSB of payload

meta:
  ( script <name>? <script> )                ;; name a subscript
  ( input <name>? <string> )                 ;; read script or module from file
  ( output <name>? <string>? )               ;; output module to stout or file
```

`wast2json` supports all of this format except the `meta` commands. The meta
commands are not used in any of the spec tests.

## JSON format

The JSON format has the following structure:

```
{"source_filename": <string>,
 "commands": [ <commands>* ] }
```

The `source_filename` is the name of the input `.wast` file. Each command is a
JSON object. They all have the following structure:

```
{"type": <string>, "line": <number>, ...}
```

The `line` property is the line in the `.wast` file where this command is
defined. The `type` property can be one of the following. Given the string, the
rest of the command object has the given structure:

| type | extra properties |
| - | - |
| "module" | `{..., "name": <string>, "filename": <string>}` |
| "action" | `{..., "action": <action>}` |
| "assert_return" | `{..., "action": <action>, "expected": <expected>}` |
| "assert_exhaustion" | `{..., "action": <action>, "text": <string>}` |
| "assert_trap" | `{..., "action": <action>, "text": <string>}` |
| "assert_invalid" | `{..., "filename": <string>, "text": <string>, "module_type": <module_type>}` |
| "assert_malformed" | `{..., "filename": <string>, "text": <string>, "module_type": <module_type>}` |
| "assert_uninstantiable" | `{..., "filename": <string>, "text": <string>, "module_type": <module_type>}` |
| "assert_unlinkable" | `{..., "filename": <string>, "text": <string>, "module_type": <module_type>}` |
| "register" | `{..., "name": <string>, "as": <string>}` |

### Actions

An action represents the wast `action`: either an "invoke" or a "get" command.
An action can be run with or without an associated assertion.

If this is an "invoke" action, the "field" property represents the name of the
exported function to run. If this is a "get" action, the "field" property
represents the name of the exported global to access.

An action has an optional module name. If the name is provided, this is the
module to run the action on. If the name is not provided, the action is run on
the most recently instantiated module.

The complete format for an "invoke" action command is:

```
{
  "type": "invoke",
  ("module": <string>)?,
  "field": <string>,
  "args": <const_vector>
}
```

The "args" property represents the parameters to pass to the exported function.

The complete format for a "get" action command is:

```
{
  "type": "get",
  ("module": <string>)?,
  "field": <string>
}
```

### Const Vectors

A const vector is an array of consts.

```
[<const>]
```

### Const

A const is a constant value. It contains both a type and a value. The following
types are supported, with the given JSON format:

| type | JSON format |
| - | - |
| "i32" | `{"type": "i32", "value": <string>}` |
| "i64" | `{"type": "i64", "value": <string>}` |
| "f32" | `{"type": "f32", "value": <string>}` |
| "f64" | `{"type": "f64", "value": <string>}` |

The `reference-types` proposal adds three more valid types:

| type | JSON format |
| - | - |
| "externref" | `{"type": "externref", "value": <string>}` |
| "funcref" | `{"type": "funcref", "value": <string>}` |


The `simd` proposal adds another type, with a slightly different syntax.

```
{
  "type": "v128",
  "lane_type": "i8" | "i16" | "i32" | "f32" | "f64",
  "value": [ (<string>,)* ]
}
```

All numeric value are stored as strings, since JSON numbers are not guaranteed
to be precise enough to store all Wasm values. Values are always written as
decimal numbers. For example, the following const has the type "i32" and the
value `34`.

```
("type": "i32", "value": "34"}
```

For floats, the numbers are written as the decimal encoding of the binary
representation of the number. For example, the following const has the type
"f32" and the value `1.0`. The value is `1065353216` because that is equivalent
to hexadecimal `0x3f800000`, which is the binary representation of `1.0` as a
32-bit float.

```
("type": "f32", "value": "1065353216"}
```

A "v128" value stores each of its lanes as a separate string. The number of
lanes depends on the "lane_type" property:

| "lane_type" | #lanes |
| - | - |
| "i8" | 16 |
| "i16" | 8 |
| "i32" | 4 |
| "i64" | 2 |
| "f32" | 4 |
| "f64" | 2 |

For example, a "v128" const with lane type "i32" and lanes `0`, `1`, `2`, `3`
would be written:

```
{
  "type": "v128",
  "lane_type": "i32",
  "value": ["0", "1", "2", "3"]
}
```

### Expected

Expected values are similar to a const vector. They are always arrays, and they
typically contain const values. For example, the following expected value is
the same as a const vector, and has the type "i32" and the value "1":

```
[
  {"type": "i32", "value": "1"}
]
```

However, an expected value can also contain a pattern. In particular, for "f32"
and "f64" types (and simd lanes), the pattern can also be "nan:canonical" or
"nan:arithmetic". For example, the following expected value has the type "f32"
and must be a canonical NaN:

```
[
  {"type": "f32", "value": "nan:canonical"}
]
```

The `simd` proposal extends this pattern for multiple lanes, where each lane
can be a const or a pattern. For example, the following expected value has four
lanes, each of type "f32", two of which have const values (lanes 0 and 2), and
two of which are patterns (lanes 1 and 3):

```
[
  {
    "type": "v128",
    "lane_type": "f32",
    "value": [
      "0",
      "nan:arithmetic",
      "0",
      "nan:arithmetic",
    ]
  }
]
```

The `multi-value` proposal allows for multiple return values from a function,
which is why the expected values is always an array. For example, the following
example has two expected values:

```
[
  {"type": "i32", "value": "1"},
  {"type": "i64", "value": "2"}
]
```

### "module" command

The "module" JSON command represents the wast `module` command. It compiles and
instantiates a new module, which then becomes the default target for future
assertions.

The complete format for the "module" command is:

```
{
 "type": "module",
 "line": <number>,
 ("name": <string>,)?
 "filename": <string>
}
```

Optionally, the module can be given a name via the "name" property. If given,
this name can be used in action commands to refer to this module instead of the
most recently instantiated one.

The "filename" property specifies the path to the `.wasm` file for this module.
The path is always relative to the JSON file.

### "action" command

```
{
 "type": "action",
 "line": <number>,
 "action": <action>
}
```

The "action" JSON command represents a wast action (either `invoke` or `get`)
that does not have an associated assertion).

### "assert_return" command

```
{
 "type": "assert_return",
 "line": <number>,
 "action": <action>
 "expected": <expected>
}
```

The "assert_return" JSON command represents the wast `assert_return` command.
It runs an action and checks that the result is an expected value.

### "assert_exhaustion" command

```
{
 "type": "assert_exhaustion",
 "line": <number>,
 "action": <action>,
 "text": <string>
}
```

The "assert_exhaustion" JSON command represents the wast `assert_exhaustion`
command. It runs an action, and checks whether this produces a stack overflow.

The "text" property specifies the text expected by the rerefence interpreter.

### "assert_trap" command

```
{
 "type": "assert_trap",
 "line": <number>,
 "action": <action>,
 "text": <string>
}
```

The "assert_trap" JSON command represents the action form of the wast
`assert_trap` command. The wast format also has an `assert_trap` that operates
on a module which is called "assert_uninstantiable" in the JSON format.

The "assert_trap" command runs an action with the expectation that it will
trap.

The "text" property specifies the text expected by the rerefence interpreter.

### "assert_invalid" command

```
{
  "type": "assert_invalid",
  "line": <number>,
  "filename": <string>,
  "text": <string>,
  "module_type": "binary" | "text"
}
```

The "assert_invalid" JSON command represents the wast `assert_invalid` command.
It reads a module with the expectation that it will not validate.

The "filename" property specifies the path to the `.wasm` or `.wat` file for
this module. The path is always relative to the JSON file.

The "text" property specifies the error text expected by the reference
interpreter.

The "module_type" property specifies whether this module is in text or binary
format.

### "assert_malformed" command

```
{
  "type": "assert_malformed",
  "line": <number>,
  "filename": <string>,
  "text": <string>,
  "module_type": "binary" | "text"
}
```

The "assert_malformed" JSON command represents the wast `assert_malformed`
command. It reads a module with the expectation that it is malformed. Note that
this is different than being invalid; a malformed module is one that doesn't
obey the syntax rules of the binary or text format.

The "filename" property specifies the path to the `.wasm` or `.wat` file for
this module. The path is always relative to the JSON file.

The "text" property specifies the error text expected by the reference
interpreter.

The "module_type" property specifies whether this module is in text or binary
format.

### "assert_uninstantiable" command

```
{
  "type": "assert_uninstantiable",
  "line": <number>,
  "filename": <string>,
  "text": <string>,
  "module_type": "binary" | "text"
}
```

The "assert_uninstantiable" JSON command represents the module form of the wast
`assert_trap` command. The wast format also has an `assert_trap` command that
runs an action, which is called "assert_trap" in the JSON format.

An uninstantiable module is one where the linking step succeeds, but the start
function traps.

The "filename" property specifies the path to the `.wasm` or `.wat` file for
this module. The path is always relative to the JSON file.

The "text" property specifies the error text expected by the reference
interpreter.

The "module_type" property specifies whether this module is in text or binary
format.

### "assert_unlinkable" command

```
{
  "type": "assert_unlinkable",
  "line": <number>,
  "filename": <string>,
  "text": <string>,
  "module_type": "binary" | "text"
}
```

The "assert_unlinkable" JSON command represents the wast `assert_unlinkable`
command. It reads a module with the expectation that it will not link with the
other modules provided; i.e., one of its imports is not provided or does not
match.

The "filename" property specifies the path to the `.wasm` or `.wat` file for
this module. The path is always relative to the JSON file.

The "text" property specifies the error text expected by the reference
interpreter.

The "module_type" property specifies whether this module is in text or binary
format.

### "register" command

```
{
  "type": "register",
  ("name": <string>,)?
  "as": <string>
}
```

The "register" JSON command represents the wast `register` command. It
registers the exports of the given module as a given name.

The "name" property specifies the module to be registered. If it is empty, the
most recently instantiated module is used.

The "as" property specifies the name to use in the registry. For example, if a module is registered as:

```
{
  "type": "register",
  "as": "my_module"
}
```

Then its exports can be imported as:

```wasm
(import "my_module" "my_export" ...)
```

## Full Example

Assume we have the following `.wast` file:

```wasm
(module
  (func (export "add") (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.add)

  (func (export "trap")
    unreachable)
)

(assert_return (invoke "add" (i32.const 11) (i32.const 22)) (i32.const 33))

(assert_trap (invoke "trap") "unreachable")

(assert_malformed
  (module quote "(modulee)")
  "syntax error"
)
```

The following JSON file will be generated (with added whitespace for clarity):

```
{
  "source_filename": "example.wast",
  "commands": [
    {
      "type": "module",
      "line": 1,
      "filename": "example.0.wasm"
    },

    {
      "type": "assert_return",
      "line": 11,
      "action": {
        "type": "invoke",
        "field": "add",
        "args": [
          {"type": "i32", "value": "11"},
          {"type": "i32", "value": "22"}
        ]
      },
      "expected": [
        {"type": "i32", "value": "33"}
      ]
    },

    {
      "type": "assert_trap",
      "line": 13,
      "action": {
        "type": "invoke",
        "field": "trap",
        "args": []
      },
      "text": "unreachable",
    },

    {
      "type": "assert_malformed",
      "line": 16,
      "filename": "example.1.wat",
      "text": "syntax error",
      "module_type": "text"
    }
  ]
}
```
