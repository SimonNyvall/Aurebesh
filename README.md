# MudShell

## Project Overview
**MudShell** is a custom-built shell written in C++ that aims to replicate the functionality of a Unix-like command-line interface. It provides a simplified command execution environment where users can run both built-in and external commands, manage input/output through piping, and handle directories using commands like `cd`. MudShell was created as a learning project to understand and implement core shell functionalities from the ground up, with plans for future enhancements.

## Features
* **Built-In Commands**
    * `cd` - Change directory
    * `exit` - Exit the shell
    * `help` - Display help information
* **External Commands**
    * Execute any valid system command (e.g., `ls`, `grep`, `cat`) by forking child processes and using `execvp`
* **Piping**
    * Connect multiple commands together using pipes (e.g., `ls |> grep .cpp`)
* **Interactive Prompt**
    * Displays the current working directory.

## Planned Features
Future updates to **MudShell** will include several powerful features that are often found in modern shells:

* Command History with Up/Down Arrow Keys: Allow users to navigate through previously executed commands using arrow keys.
* Tab Completion: Enable auto-completion of commands and file paths with the Tab key, ignoring case sensitivity.
* Advanced Piping: Extend piping functionality to include more complex scenarios, like output redirection and background jobs.
* Back-Piping: Implement back-piping, where the output of one command can be passed back to a previous command.
* Fast Directory Switching (z-like behavior): Implement a smart directory-switching feature similar to z, which allows quick access to frequently visited directories.
* Alias Support: Allow users to define shortcuts for long commands (e.g., aliasing ll to ls -la).
* Arrow Keys for Cursor Movement: Enhance the input field to support left/right arrow keys for editing commands, as well as Home and End key support.