# Phooey

Phooey is an extended version of [Foo](https://esolangs.org/wiki/Foo) written in C++.

## Memory layout

There are two primary sources of memory:

1. A array of integers (much like [BF](https://esolangs.org/wiki/Brainfuck )'s)
2. A stack

## Commands

All commands have _arguments_, which are either explicit or taken from the program's state. For example, `$`, the output command, takes its input from the array: `&10 $i` oututs `10`. Most commands can be used to take an argument as well: `$i10` does the same thing as before.

| command | number of specifiers | Argument source | description |
|------|----|-|-|
| `$` | 1 | Array | Performs some kind of output, depending on the specifier. <br/> `$i` - decimal output <br/> `$x` - hexadecimal output <br/> `$c` - character output. |
| `"` | 0 | - | Outputs characters until another `"` is met, or EOF. |
| `<` | 0 | Constant (1) | Moves the array pointer a certain number of spaces left. |
| `>` | 0 | Constant (1) | Moves the array pointer a certain number of spaces right. |
| `@` | 0 | Array | Pushes the argument to the stack. |
| `&` | 0 | Stack | Sets the current cell to the argument. |
| `#` | 0 | Array | Sleeps for the number of seconds specified by the argument. |
| `(` | 0 | Constant (0) | 