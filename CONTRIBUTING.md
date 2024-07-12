# Contributing to Magnesia
Contributions to the Magnesia project are welcome.
If you want to contribute this file defines the rules to follow.

## Issues
If you encounter a problem with this project you can create an issue.

## Merge Requests
You can contribute through merge requests (MRs).
The following requirements must be met for a merge request:
- **Continuous integration (CI):** every MR must pass our CI, which includes compilation on all targeted architectures, passing unit tests.
- **Code style:** See [Code Style](#code-style) for further clarification.
- **Code format:** The code must be formatted according to clang-format
- **Commit messages:** See [Commit Conventions](#commit-conventions) for further clarification.

## Commit Conventions
We use [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) extended with custom commit types based on
[Angular's Commit Message Guidelines](https://github.com/angular/angular/blob/22b96b9/CONTRIBUTING.md#-commit-message-guidelines).
Each commit message consists of a **Header**, **Body** and **Footer**. The Header is mandatory and must consist of a type, a scope and a subject.
```
<type>(<scope>): <subject>
<BLANK LINE>
<body>
<BLANK LINE>
<footer>
```
### Footer
The footer must include a closing reference to an issue if available.

### Types
The following Types are allowed:
- **feat:** A new feature
- **fix:** A bug fix
- **refactor:** A code change that neither fixes a bug nor adds a feature
- **style:** Changes that do not affect the meaning of the code (white-space, formatting, missing semi-colons, etc)
- **test:** Adding missing tests or correcting existing tests
- **chore:** i.e. dependency updates
- **build:** Changes that affect the build system or external dependencies
- **perf:** A code change that improves performance
- **ci:** Changes to our CI configuration files and scripts

### Scopes
Only the following Scope are allowed to use:
- **repo-docs**: everything that only touches the repository documentation files (e.g., `README.md`, `CONTRIBUTING.md`)
- **cmake**: mainly used with the **build** type for everything that touches CMake files
- **cmake-format**: changes to the `.cmake-format.yaml` file
- **clang-format**: changes to the `.clang-format` file
- **clang-tidy**: changes to the `.clang-tidy` file
- **docs**: changes related to code docs
- **linux**: changes to the linux ci configuration
- ...

To define a new scope add it above.

### Subject
The subject must abide by the following rules:
- must contain a succinct description of change
- written in imperative present tense: "change" not "changed" nor "changes"
- no capitalization of the first letter
- no dot (.) at the end
- For breaking changes add a `!` after the type/scope.

### Body
The body must use imperative present tense as described [Subject](#subject) and should contain the motivation of the change in contrast to the previous
behavior.

## Code Style
Create a cpp file for each header file.
Do so even when the cpp file only includes the header.

### Naming-Conventions
We use the following naming conventions:
- **Classes:** PascalCase
- **Enums:** PascalCase
- **Member functions (includes signals and slots):** camelCase()
- **Private member variables:** m_snake_case
- **Public member variables:** snake_case
- **Static member variables:** s_snake_case
- **Free functions:** snake_case
- **Parameters:** snake_case
- **Other variables:** snake_case
- **Files:**
    - PascalCaseClass.cpp
    - PascalCaseClass.hpp
    - snake_case.cpp
    - snake_case.hpp
- **Dirs:** snake_case
- **Namespaces:** snake_case
- **Namespaces:**
    - magnesia
    - one per activity

### Directory Structure
Create a subdirectory for each namespace with its own files.

<!-- TODO -->
<!-- ### Docs -->

<!-- TODO -->
<!-- ### Tests -->
