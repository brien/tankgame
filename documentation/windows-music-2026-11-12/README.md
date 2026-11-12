# Windows music stops immediately

The supplied Windows log reports SDL 2.32.10, SDL_mixer 2.8.2 and WASAPI.
The Ogg loads and starts, but by tick 632 it is no longer playing. Position
remains at 0.032 seconds, despite a duration of 178.312 seconds and both
music volume readings at 128/128. Pause/resume does not restart stopped music.

## Reproduction

An isolated Linux probe using upstream SDL 2.32.10 and SDL_mixer 2.8.2
(STB Vorbis backend) reproduces the exact stop with the repository's
`runtime/sounds/TGTest3.ogg`. It uses SDL's dummy audio driver, so no renderer,
gameplay, input, Windows mixer settings or physical audio hardware is involved.

| Requested rate | Buffer frames | Playing after 1.2 s | Reported position |
| --- | --- | --- | --- |
| 22050 | 64 | No | 0.016 s |
| 22050 | 128 | No | 0.032 s |
| 22050 | 256 | Yes | 1.277 s |
| 22050 | 2048 | Yes | 1.161 s |
| 44100 | 128 | Yes | 1.654 s |

Positions are decoder positions, not precise wall-clock measurements. The
dummy driver can run faster than real time for small buffers. The finished
callback ran on the failing cases with an empty audio-thread error string.
The system SDL2 compatibility library (2.32.64) did not reproduce this failure;
use classic SDL 2.32.10 for the reproduction.

`music-probe.cpp` can be built against the chosen SDL2 and SDL2_mixer headers
and libraries, then run from the repository root:

```sh
music-probe 22050 128 runtime/sounds/TGTest3.ogg
music-probe 44100 128 runtime/sounds/TGTest3.ogg
```

Ensure the runtime loader also selects those library builds. Local reproduction
used upstream release archives, built outside the repository under `/tmp`.

## Mechanism and change

SDL_mixer's `music_pcm_getaudio` stops after more than ten consecutive decoder
calls produce no output. SDL2's audio stream buffers input before downsampling
this 44100 Hz track to the requested 22050 Hz. With small buffers, the retry
limit is reached before converted output is available. The music mixer then
marks playback as stopped, even though the track has not ended.

Upstream source:

- [SDL_mixer retry limit and stop handling](https://github.com/libsdl-org/SDL_mixer/blob/release-2.8.2/src/music.c)
- [STB Ogg decoding and conversion stream](https://github.com/libsdl-org/SDL_mixer/blob/release-2.8.2/src/codecs/music_ogg_stb.c)
- [SDL2 audio stream staging and resampling](https://github.com/libsdl-org/SDL/blob/release-2.32.10/src/audio/SDL_audiocvt.c)

`SoundTask::Start` now requests the existing `samplingRate` value of 44100 Hz
instead of hardcoding 22050 Hz. This matches the music asset and avoids that
downsampling path when the requested rate is obtained. Existing WAV effects
remain handled by SDL_mixer's sample conversion. No asset changes are needed.

The exact Windows device buffer size and decoder backend were not logged, so
the reproduction strongly supports this explanation without establishing those
two details on the target machine. A Windows run is still required to confirm
audible playback after the change. Check that its startup audio rate is 44100 Hz
and that progress remains `playing=1` with an advancing position.

Validation: native application/tests build succeeded; the existing
`BrowserLifecycleTest.AudioActivationAndFocusPreserveRequestedMusicPause` test
passed. The isolated 128-frame probe stopped at 0.032 s with the old rate and
continued playing with the new rate. Windows, macOS and browser audio have not
been revalidated for this change.
