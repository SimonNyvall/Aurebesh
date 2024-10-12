# MudShell

MudShell is a modern, user-friendly command line shell for **Linux** systems. Inspired by Unix-like shells and the F# programming language. MudShell brings powerful features like command history, piping (forward and backward), **Git** integration into the prompt, fast **cd** command (similar to **zoxide**) and more.

## Quick Start
MudShell funxtions similarly to other popular shells like Bash and Zsh. Key differences include its interative prompt.

To get started, tun the following commands to build and install MudShell;
```bash
make
```

To run MudShell, simply run the following command;
```bash
./mudshell
```

To clean the build artifacts:
```bash
make clean
```

## Features
Built-in Commands

MudShell comes with several built-in commands, including:

* `cd`: Change directory.
* `exit`: Exit the shell.
* `help`: Display help information about available commands.

## External Commands

MudShell fully supports external system commands. For example, you can run commands like ls, grep, and cat using child processes with execvp.
Piping

MudShell allows you to connect multiple commands using pipes, passing the output of one command to the next (e.g., ls |> grep .cpp).
Command History

Navigate through your command history with the Up/Down arrow keys. This makes it easier to rerun or modify previous commands.
Interactive Prompt

MudShell's prompt is customizable and displays the current working directory. It also integrates with Git, similar to the Starship prompt, to show repository status.
Input Support

MudShell supports left/right arrow keys for cursor movement, as well as Home/End keys for quick navigation. You can edit commands directly within the input line.
Planned Features

MudShell is continuously evolving. Planned features for future releases include:

* `Tab Completion`: Autocomplete commands and file paths with the Tab key.
* `Back Piping`: Pass command output back to previous commands.
* `Fast Directory Switching`: Introduce smart directory-switching similar to the zoxide tool for quickly accessing frequently visited directories.
* `Alias Support`: Allow users to create shortcuts for commonly used commands (e.g., alias ll for ls -la).
* `Advanced Piping and Redirection`: Support more complex piping and redirection scenarios, such as output redirection (>), appending (>>), and background jobs (&).

## Dependencies

Running MudShell requires:

* Basic POSIX utilities such as cat, cut, ls, rm, and mkdir.
* Ncurses: Provides terminal handling functionalities (usually pre-installed on most Linux distributions). 
* Git: Required for Git integration in the prompt.