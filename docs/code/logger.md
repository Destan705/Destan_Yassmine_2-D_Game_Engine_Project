# Logger — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/platform/logger.h`](../../include/platform/logger.h) and [`src/platform/logger.cpp`](../../src/platform/logger.cpp).

**In one sentence:** Prints a message with a `[INFO]`, `[WARNING]` or `[ERROR]` label: errors go to standard error and everything else to standard output.

**Why it exists:** One consistent way to report what the engine is doing, so messages are easy to spot and filter.

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `logger_level` | Severity | [Enum](#logger-level) |
| `loggerLevelToString` | Level → label text | [Function](#loggerleveltostring) |
| `logMessage` | Print a labelled message | [Function](#logmessage) |

## State and types

<a id="state-and-types"></a>

| Name | Kind / type | Value or initialization | What it means / where used |
| --- | --- | --- | --- |
| <a id="logger-level"></a>`logger_level` | `enum class` | `Info`, `Warning`, `Error` | Severity of a message |

## Functions

<a id="loggerleveltostring"></a>
### `std::string loggerLevelToString(logger_level level)`

**What it does:** Returns `"INFO"`, `"WARNING"` or `"ERROR"`, or `"UNKNOWN"` for any other value. It's an internal helper that isn't in the header. **No direct project-function calls.**

<a id="logmessage"></a>
### `void logMessage(logger_level level, const std::string& message)`

**What it does:** Writes `[LABEL] message` followed by a newline.

| Name | Type | Meaning |
| --- | --- | --- |
| `level` | `logger_level` | Severity, which decides the label and the stream |
| `message` | `const std::string&` | Text, printed exactly as given (spaces and tabs kept) |

**What happens:** gets the label from [`loggerLevelToString`](#loggerleveltostring). `Error` writes to `std::cerr`; `Info` and `Warning` write to `std::cout`. Uses `std::endl` (which flushes).

**Used by:** [platform](platform.md), [Renderer](renderer.md), [engine](engine.md). **Related tests:** [LoggerTest](../tests/test-logger.md).

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
