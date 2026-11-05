# test_audio_service.cpp — Test Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728`
>
> **Test source:** [`tests/audio/test_audio_service.cpp`](../../tests/audio/test_audio_service.cpp)

**In one sentence:** It checks [AudioService](../code/audio-service.md) loading, caching, limits, playback and the no-device fallback, using SDL's `dummy` audio driver.

**Why these tests matter:** Sound must never crash the game. These tests prove every failure becomes `0` or `false`, and that the cache and limits work, without needing speakers.

## Setup and shared helpers

| Item | What it does |
| --- | --- |
| `WriteWav(path, sampleRate, sampleCount)` | Writes a minimal 16-bit mono PCM WAV of silence. Duration = `sampleCount / sampleRate` seconds. |
| `MakeTempFolder()` | Creates a fresh random folder in the system temp directory |
| `AudioServiceTest::SetUp()` | Forces the `dummy` audio driver, `SDL_Init(SDL_INIT_AUDIO)`, creates the temp root with `audio/click.wav` (0.1 s) and `audio/beep.wav` (0.2 s) |
| `AudioServiceTest::TearDown()` | `SDL_Quit`, resets the hint, deletes the folder |

## Individual tests

### Device

<a id="audioservicetest-isavailablewithworkingdevice"></a>
#### `AudioServiceTest.IsAvailableWithWorkingDevice`

With the dummy device, [`isAvailable`](../code/audio-service.md#audioservice-isavailable) is true.

<a id="audioservicetest-canbecreatedanddestroyedrepeatedly"></a>
#### `AudioServiceTest.CanBeCreatedAndDestroyedRepeatedly`

Three services in a row each open and close the device cleanly.

### Loading and caching

<a id="audioservicetest-loadsvalidwav"></a>
#### `AudioServiceTest.LoadsValidWav`

[`loadWav`](../code/audio-service.md#audioservice-loadwav) on `click.wav` returns a non-zero ID.

<a id="audioservicetest-samepathreturnssameid"></a>
#### `AudioServiceTest.SamePathReturnsSameId`

Loading twice returns the same ID.

<a id="audioservicetest-equivalentpathspellingsshareoneid"></a>
#### `AudioServiceTest.EquivalentPathSpellingsShareOneId`

`./audio/click.wav` and `audio\click.wav` return the same ID.

<a id="audioservicetest-differentfilesgetdifferentids"></a>
#### `AudioServiceTest.DifferentFilesGetDifferentIds`

`click` and `beep` get different IDs.

### Failures (return 0, never throw)

<a id="audioservicetest-missingfilereturnszero"></a>
#### `AudioServiceTest.MissingFileReturnsZero`

A missing file returns 0.

<a id="audioservicetest-failedloadisnotcachedandcanberetried"></a>
#### `AudioServiceTest.FailedLoadIsNotCachedAndCanBeRetried`

`later.wav` fails, is then written, and loads on retry.

<a id="audioservicetest-nonwavfilereturnszero"></a>
#### `AudioServiceTest.NonWavFileReturnsZero`

A text file named `.wav` returns 0.

<a id="audioservicetest-soundlongerthanfivesecondsisrejected"></a>
#### `AudioServiceTest.SoundLongerThanFiveSecondsIsRejected`

A 6-second WAV returns 0.

<a id="audioservicetest-emptysoundisrejected"></a>
#### `AudioServiceTest.EmptySoundIsRejected`

A WAV with no samples returns 0.

<a id="audioservicetest-invalidpathsreturnzero"></a>
#### `AudioServiceTest.InvalidPathsReturnZero`

Empty, `.`, rooted, absolute, drive-letter and `..` paths all return 0.

<a id="audioservicetest-failedloadsdonotuseupids"></a>
#### `AudioServiceTest.FailedLoadsDoNotUseUpIds`

After two failed loads, the next good file gets `first + 1`: failures don't consume IDs.

### Playback

<a id="audioservicetest-playsloadedsound"></a>
#### `AudioServiceTest.PlaysLoadedSound`

[`play`](../code/audio-service.md#audioservice-play) returns true for a loaded sound.

<a id="audioservicetest-playingagainrestartswithouterror"></a>
#### `AudioServiceTest.PlayingAgainRestartsWithoutError`

Playing the same sound twice, then another, all succeed (each replaces the last).

<a id="audioservicetest-playingunknownidfails"></a>
#### `AudioServiceTest.PlayingUnknownIdFails`

`play(0)` and `play(12345)` return false.

<a id="audioservicetest-stopissafebeforeandafterplaying"></a>
#### `AudioServiceTest.StopIsSafeBeforeAndAfterPlaying`

[`stop`](../code/audio-service.md#audioservice-stop) never throws, even twice in a row.

### No audio device

<a id="audioservicewithoutdevicetest-degradesgracefully"></a>
#### `AudioServiceWithoutDeviceTest.DegradesGracefully`

SDL audio is **not** initialised. `isAvailable` is false, `play` returns false, `stop` doesn't throw. Loading may still work.

## Running these tests

**Target:** `engine_tests`. **Filter:** `ctest --test-dir build -R AudioService`. The `dummy` driver is forced in code, so no environment variable is needed.

## Coverage gaps

Not tested: the 2 MB per-file and 8 MB cache limits (would need large files), running out of IDs, and whether sound is actually audible (the dummy driver discards it).

**Back to:** [Test index](index.md) · [Code index](../code/index.md)
