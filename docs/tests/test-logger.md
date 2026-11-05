# test_logger.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/platform/test_logger.cpp`](../../tests/platform/test_logger.cpp)

**In one sentence:** It checks the exact text and the output stream of [`logMessage`](../code/logger.md#logmessage) for each level.

## Setup and shared helpers

| Member / method | What it does | Why |
| --- | --- | --- |
| `output`, `errors` | `std::ostringstream` buffers | Capture what was printed |
| `SetUp()` | Redirects `std::cout` → `output` and `std::cerr` → `errors` (saving the originals) | Inspect the output |
| `TearDown()` | Restores both streams | Other tests print normally |

## Individual tests

<a id="loggertest-infousescorrectlabelandstandardoutput"></a>
### `LoggerTest.InfoUsesCorrectLabelAndStandardOutput`

Info prints `"[INFO] Engine started\n"` on stdout, with nothing on stderr.

<a id="loggertest-warningusescorrectlabelandstandardoutput"></a>
### `LoggerTest.WarningUsesCorrectLabelAndStandardOutput`

Warning prints `[WARNING] …` on **stdout**.

<a id="loggertest-errorusescorrectlabelandstandarderror"></a>
### `LoggerTest.ErrorUsesCorrectLabelAndStandardError`

Error prints `[ERROR] …` on **stderr**, with nothing on stdout.

<a id="loggertest-emptymessagestillincludeslabelandnewline"></a>
### `LoggerTest.EmptyMessageStillIncludesLabelAndNewline`

An empty message prints `"[INFO] \n"`.

<a id="loggertest-consecutivemessagesremainseparateandusecorrectstreams"></a>
### `LoggerTest.ConsecutiveMessagesRemainSeparateAndUseCorrectStreams`

Four mixed messages. Each lands on its correct stream, in order, on separate lines.

<a id="loggertest-messagepunctuationandwhitespacearepreserved"></a>
### `LoggerTest.MessagePunctuationAndWhitespaceArePreserved`

Leading and trailing spaces, tabs, brackets and `%` are printed unchanged.

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
