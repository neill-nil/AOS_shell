# Custom Linux Shell

This project is a simple custom Linux shell built in C++ which replicates core features of a standard shell.
It supports basic shell commands like cd, ls, pwd, I/O redirection, pipelines, signals (Ctrl-C, Ctrl-Z, Ctrl-D), command history, autofill; Some new commands like pinfo, background tasks, etc. 

---

## Project Structure

### `src/`
- **main.cpp**  
  Entry point of the shell. Handles user input, integrates history, and runs the main event loop.

- **utils.cpp**  
  Contains helper functions like `tokenize`, string splitting, prompt formatting, and other utility logic.

- **dispatcher.cpp**  
  Acts as the command router. Dispatches parsed commands to the appropriate handler function.

- **commands.cpp**  
  Implements all shell commands (`cd`, `ls`, `echo`, `pwd`, `history`, I/O redirection, pipelines, etc.).

### `include/`
- **utils.hpp**  
  Header file for helper functions (tokenizer, string utils, etc.).

- **dispatcher.hpp**  
  Header for the dispatcher function declarations.

- **commands.hpp**  
  Header declaring all built-in command implementations.

---

## Running

Use Makefile provided

Compile: Run `make` in your terminal
Run: `./CustomShell` in your terminal

Then you can write desired commands