# Go-- Compiler

A hybrid compiler for Go--, a fictional programming language, built as a university project.

## About

This is a hybrid compiler I wrote for my Compiler Construction course at university. It compiles Go-- (a made-up language inspired by Go) into machine code and executes it on a built-in virtual machine. The full pipeline runs through four stages: lexical analysis, parsing, translation to three-address code, and VM execution.

See [Documentation.pdf](documentation/Documentation.pdf) for the complete language specification and CFGs.

## Compiler Pipeline

```
Source Code (.go)
    ↓
Lexical Analyzer  →  words.txt (tokens)
    ↓
Parser            →  parsetree.txt, parser-symboltable.txt
    ↓
Translator        →  tac.txt (three-address code), translator-symboltable.txt, machine-code.txt
    ↓
Virtual Machine   →  executes machine-code.txt
```

## Go-- Language

| Feature | Syntax |
|---|---|
| Data types | `Integer`, `char` |
| Variables | `Integer: x, y;` |
| Assignment | `x := 5;` |
| Decisions | `if`, `elif`, `else` |
| Loop | `while condition: { ... }` |
| Functions | `func Integer: name (Integer: param) { ... }` |
| I/O | `In >> x;`, `print(x)`, `println(x)` |
| Return | `ret` |
| Operators | `+ - * /`, `< <= > >= = /=` |
| Comments | `/* ... */` |

Example (`test/test.go`):
```
func Integer: numPrint (Integer: num, Integer: length)
{
    Integer: i, j, k;
    In >> i;

    j := 0;
    while j < i:
    {
        k := i / 2;
        if j < k:{
            println(j);
        }
        else {
            k := j - 1;
            println(k);
        }
        j := j + 1;
    }
}
```

## Built With

- C++14

## Getting Started

### Prerequisites

- A C++ compiler with C++14 support (clang++, g++, MSVC)

### Building & Running

```sh
git clone https://github.com/mosamaasif/Gomm_Compiler.git
cd Gomm_Compiler
```

On macOS/Linux:
```sh
clang++ -std=c++14 -I includes src/*.cpp -o gomm
./gomm
```

On Windows (MSVC Developer Command Prompt):
```sh
cl /std:c++14 /I includes src\*.cpp /Fe:gomm.exe
gomm.exe
```

The compiler will prompt for a source file path. Point it to `test/test.go` to try it out. Output files (`words.txt`, `parsetree.txt`, `tac.txt`, `machine-code.txt`, etc.) are written to the current working directory.
