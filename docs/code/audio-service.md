# AudioService — Source Reference

> **Coverage:** Verified · **Source revision:** `phase3` @ `ca2b728` · **Last checked:** 2026-11-05
>
> **Files:** [`include/audio_service.h`](../../include/audio_service.h) and [`src/audio_service.cpp`](../../src/audio_service.cpp).

**In one sentence:** Loads short WAV files into memory once and plays one of them at a time through SDL's default audio device.

**Why it exists:** It hides every SDL audio call behind four simple functions, and it **never crashes the game** over sound: every failure is logged and reported with `0` or `false`.

## At a glance

| This code… | Explanation |
| --- | --- |
| Is responsible for | WAV validation, the sound cache, one playback stream |
| Gets information from | WAV files under its asset root |
| Changes | Its cache; the audio stream (what you hear) |
| Used by | [engine](engine.md#engine) (loads `audio/test.wav`, plays it when the player enters the trigger), tests |
| Depends on | SDL3 audio, [logger](logger.md) |

## Contents

| Symbol | What it means | Jump |
| --- | --- | --- |
| `AudioService` | Sound cache + player | [Class](#class-audioservice) |
| `SoundId` | `std::uint32_t`; `0` = invalid | [Type](#audioservice-soundid) |
| `AudioService(path)` / `~AudioService()` | Open / close the device stream | [Constructor](#audioservice-audioservice) · [Destructor](#audioservice-destructor) |
| `loadWav(path)` | Load and cache a WAV | [Function](#audioservice-loadwav) |
| `play(SoundId)` | Play from the start | [Function](#audioservice-play) |
| `stop()` | Stop playing | [Function](#audioservice-stop) |
| `isAvailable()` | Is there a working device? | [Function](#audioservice-isavailable) |
| `Impl`, limits, helpers | Private details | [Internals](#internals) |

## Class: `AudioService`

<a id="class-audioservice"></a>

**How it's created:** `AudioService audio(getAssetRootPath());` **after** `SDL_Init(SDL_INIT_AUDIO)` and destroyed **before** `SDL_Quit()`. In [`engine()`](engine.md#engine) it lives in the inner scope for exactly this reason.

**Pimpl ("pointer to implementation"):** the header only has `struct Impl;` and a `std::unique_ptr<Impl> impl_`. All SDL types stay in the `.cpp`, so including `audio_service.h` doesn't pull in SDL. The destructor is defined in the `.cpp` (`= default`) because `unique_ptr` needs the full `Impl` type to delete it.

**Limits:**

| Limit | Value |
| --- | --- |
| Format | WAV (whatever `SDL_LoadWAV` accepts) |
| Length | ≤ 5 seconds |
| One file (decoded) | ≤ 2 MB |
| Whole cache | ≤ 8 MB |
| Playing at once | 1 sound; playing again restarts / replaces it |

<a id="audioservice-soundid"></a>
**`using SoundId = std::uint32_t;`** IDs start at 1 and go up by one per **successful** load. `0` always means "no sound". IDs are never reused.

## Functions

<a id="audioservice-audioservice"></a>
### `explicit AudioService::AudioService(std::filesystem::path assetRoot)`

**What it does:** Stores the root and opens the default playback device with `SDL_OpenAudioDeviceStream`, then un-pauses it with `SDL_ResumeAudioStreamDevice`.

**If there's no device** (SDL audio not initialised, no sound card…): logs a Warning and leaves the stream `nullptr`. The object is still usable; [`isAvailable()`](#audioservice-isavailable) returns `false` and `play` returns `false`. **It never throws for a missing device.**

<a id="audioservice-destructor"></a>
### `AudioService::~AudioService()`

Destroys `impl_`, whose destructor calls `SDL_DestroyAudioStream` (closing the device) if a stream exists.

<a id="audioservice-loadwav"></a>
### `SoundId AudioService::loadWav(std::filesystem::path filePath)`

**What it does:** Returns the cached ID for this path, or loads, checks and caches the WAV.

| Name | Type | What the caller supplies | Rules |
| --- | --- | --- | --- |
| `filePath` | `std::filesystem::path` | Path relative to the asset root, e.g. `"audio/test.wav"` | Same path rules as [AssetManager](asset-manager.md#assetmanager-normalizepath): not empty, not absolute/rooted, no drive letter, no `..` |

**Returns:** an ID `≥ 1`, or `0` on **any** failure (with a Warning in the log).

**What happens, in order**

1. Normalises the path (see [Internals](#internals)); invalid → `0`.
2. **Cache hit** → returns the existing ID.
3. Builds `assetRoot / path` and calls `SDL_LoadWAV`. Missing or not a WAV → `0`.
4. SDL's buffer is wrapped in a `unique_ptr` with `SDL_free`, so it's freed on every exit path.
5. `validateWav`: not empty, ≤ 2 MB, positive rate and channels, whole frames only, ≤ 5 seconds. Fails → `0`.
6. Checks the cache has room (8 MB total). Full → `0`.
7. Copies the samples into the cache, inserts into both maps (undoing the first if the second fails), increments `cachedBytes` and `nextSoundId`, returns the ID.

Any exception inside (e.g. out of memory) is caught and turned into `0`.

**Loading works without a device** (it's just file reading), so you can load first and play later.

**Calls other project functions:** [`logMessage`](logger.md#logmessage) on every failure.

**Related tests:** [SamePathReturnsSameId](../tests/test-audio-service.md#audioservicetest-samepathreturnssameid), [FailedLoadIsNotCachedAndCanBeRetried](../tests/test-audio-service.md#audioservicetest-failedloadisnotcachedandcanberetried), [SoundLongerThanFiveSecondsIsRejected](../tests/test-audio-service.md#audioservicetest-soundlongerthanfivesecondsisrejected), [InvalidPathsReturnZero](../tests/test-audio-service.md#audioservicetest-invalidpathsreturnzero).

<a id="audioservice-play"></a>
### `bool AudioService::play(SoundId soundId)`

**What it does:** Clears whatever is playing, sets the stream to the sound's format, and queues its samples, so the sound plays **from the start**.

**Returns:** `true` if queued. `false` if there's no device, the ID is unknown (including `0`), or an SDL call fails (logged).

**Why "clear first":** entering the trigger again quickly restarts the sound instead of stacking copies.

**Called by:** [`engine`](engine.md#engine) when [`handleCollisions`](game.md#handlecollisions) reports that the player just entered the trigger.

<a id="audioservice-stop"></a>
### `void AudioService::stop()`

Clears the stream (silence). Does nothing without a device. Safe to call any number of times, before or after playing.

<a id="audioservice-isavailable"></a>
### `bool AudioService::isAvailable() const`

`true` if the device stream opened. Use it to decide whether to show "sound off", for example.

## Internals

<a id="internals"></a>

| Name | Where | What it is |
| --- | --- | --- |
| `maxWavSize`, `maxCacheSize`, `maxDurationSeconds` | anonymous namespace | 2 MB, 8 MB, 5 s |
| `struct CachedSound { samples, spec }` | anonymous namespace | Raw sample bytes plus their `SDL_AudioSpec` (rate, channels, format) |
| `normalizePath(path)` | anonymous namespace | Same rules as AssetManager's, but returns `std::nullopt` + Warning instead of throwing |
| `validateWav(spec, length)` | anonymous namespace | Size, format, whole-frame and duration checks |
| `struct AudioService::Impl` | `.cpp` | `assetRoot`, `pathToId`, `idToSound`, `nextSoundId` (starts at 1), `cachedBytes`, `stream`; `insertSound` keeps both maps in sync |

## Things worth knowing

- **Errors are values, not exceptions** (unlike AssetManager, which throws). Sound is optional; a missing sound should never stop the game.
- **No unloading:** sounds stay cached until the service is destroyed.
- The path rules are duplicated from AssetManager rather than shared.
- **Tests** run on SDL's `dummy` audio driver, so no speakers are needed: [test_audio_service](../tests/test-audio-service.md).

**Back to:** [Code index](index.md) · [Symbol index](symbol-index.md)
