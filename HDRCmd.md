`HDRCmd` Command line utility
=============================
Since version 0.5, the `HDRCmd` command line utility is included. It can be used to toggle HDR on and off from scripts and check it's status.

Syntax:

    HDRCmd [SUBCOMMAND] [SUBCOMMAND-OPTIONS]

## `on` command
Turns HDR on on all supported displays.

Does not accept any options.

## `off` command
Turns HDR off on all supported displays.

Does not accept any options.

## `status` command
Prints the current HDR status to the console. Has a special mode that returns an exit code depending on the status.

### `--mode` (`-m`) option
Specifies how the status should be reported. Accepts the following values:

| `--mode` argument     | Meaning |
|-----------------------|---------|
|`short`, `s` (default) | Print a single line indicating the overall HDR status. |
|`long`, `l`            | Print the overall HDR status and status per display. |
|`exitcode`, `x`        | Special mode for scripting. Exit code is 0 if HDR is on, 1 if HDR is off, and 2 if HDR is unsupported. (Other values indicate some error.) |
