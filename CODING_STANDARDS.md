# C Coding Standards

These guidelines are for C code written for this project. They are meant to
help the team write code that is clear, safe, and easy to maintain. They do not
replace the rules used by third-party or generated code.

## Which C to use

- Write standard C11. Use POSIX.1-2008 functions when the program needs
  operating-system features, and keep platform-specific code separate where
  practical.
- Avoid compiler-specific features unless a library or platform requires them.
  Explain their use in the code.
- Add only the libraries a feature needs. Do not copy or change third-party
  code without a project need.

## Formatting and names

- Format C code with the repository's `.clang-format` file.
- Use four spaces for indentation, not tabs. Keep lines to 100 characters or
  fewer when practical.
- Use `lower_snake_case` for functions, variables, and file names. Use
  `UPPER_SNAKE_CASE` for macros and constants. For example:
  `send_frame`, `frame_count`, `video_sender.c`, and `MAX_FRAME_SIZE`.
- Use clear names that describe what something does. Mark functions and
  file-level data `static` when they are only used in that source file.
- Keep functions focused on one task. Avoid shared global variables when
  possible.

## Headers and functions

- Put the declaration of each function that other files use in a header file.
  Make headers self-contained: include the other headers they need.
- Give each header a unique include guard so it is not included more than once.
- In a `.c` file, include its matching header first, then project headers, then
  system and library headers.
- Use `const` for input values a function does not change. Use `size_t` for
  sizes and indexes, and fixed-width integer types such as `uint16_t` for
  values with a specific size, such as network fields.
- Keep function inputs and outputs simple. Document who owns a buffer, how long
  it remains valid, what its units are, and how errors are reported when that
  is not obvious.

## Memory, network data, and errors

- Every allocated block of memory or opened resource must have a clear owner
  and a matching cleanup step. Check whether allocation and resource-opening
  calls fail.
- Before using data, check its length and bounds. Check calculations for
  overflow. Treat data received from the network as untrusted: it may be
  incomplete, malformed, or laid out differently than expected.
- Read and write network fields explicitly, including their byte order. Do not
  treat raw packet bytes as if they were already a C structure.
- Report errors using the function's documented return value or status. Do not
  ignore failures or exit the program from reusable library code. Report errors
  to users at the application boundary.
- When a function acquires several resources, a shared cleanup section using
  `goto` can be a clear way to release everything exactly once.

## Threads and tests

- Protect data shared between threads, and be clear about which thread owns
  each buffer. Avoid holding a lock while waiting for input/output or calling
  another function unless necessary.
- Add tests for new behavior. For code that reads packets or other input, test
  both normal data and malformed or boundary cases.
- Keep tests repeatable and able to run without streaming hardware when
  practical.
- Build project C code with `-Wall -Wextra -Wpedantic` and fix the warnings.
  Use `-Werror` in CI when supported. Apply these options to project code, not
  automatically to third-party code.
- Run the relevant tests and formatter check for your change. When supported,
  use AddressSanitizer and UndefinedBehaviorSanitizer to help find memory and
  other runtime errors in host-side tests.
