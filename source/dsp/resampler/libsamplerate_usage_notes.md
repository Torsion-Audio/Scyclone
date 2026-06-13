# libsamplerate (Secret Rabbit Code) – usage notes

Summary of official docs and FAQs for using [libsamplerate](https://libsndfile.github.io/libsamplerate/) in Scyclone: resampling, buffers, sample rates, and latency.

---

## Contents

- [References](#references)
- [API and SRC_DATA](#api-and-src_data)
- [Transport delay and buffer sizing](#transport-delay-and-buffer-sizing)
- [end_of_input and streaming](#end_of_input-and-streaming)
- [Quality and converter type](#quality-and-converter-type)
- [Ratio and numeric precision](#ratio-and-numeric-precision)
- [Other practical points](#other-practical-points)
- [Relation to Scyclone](#relation-to-scyclone)

---

## References


| Resource     | URL                                                                       |
| ------------ | ------------------------------------------------------------------------- |
| **Full API** | [api_full.html](https://libsndfile.github.io/libsamplerate/api_full.html) |
| **FAQ**      | [faq.html](https://libsndfile.github.io/libsamplerate/faq.html)           |
| **Quality**  | [quality.html](https://libsndfile.github.io/libsamplerate/quality.html)   |
| **GitHub**   | [libsndfile/libsamplerate](https://github.com/libsndfile/libsamplerate)   |


---

## API and SRC_DATA

**Use the Full API**, not `src_simple`: [Full API](https://libsndfile.github.io/libsamplerate/api_full.html) — `src_new()` → `src_process()` in a loop → `src_delete()`. Required for streaming; [FAQ Q4/Q5](https://libsndfile.github.io/libsamplerate/faq.html) — `src_simple()` is whole-file only and must not be used on successive blocks.

**SRC_DATA** (caller sets):


| Field                       | Meaning                                                           |
| --------------------------- | ----------------------------------------------------------------- |
| `data_in`, `input_frames`   | Input buffer and length (**frames** = samples × channels)         |
| `data_out`, `output_frames` | Output buffer and **maximum** length (frames)                     |
| `src_ratio`                 | **output_sample_rate / input_sample_rate**                        |
| `end_of_input`              | `0` while more input available, `1` when flushing (end of stream) |


**After `src_process()`:** use `**output_frames_gen`** (frames written) and `**input_frames_used**` (frames consumed). Buffers must not overlap.

---

## Transport delay and buffer sizing

See [FAQ Q6](https://libsndfile.github.io/libsamplerate/faq.html).

- SINC converters have **internal transport delay**. First call(s) may produce **fewer** output frames than `input_frames * src_ratio`; later calls settle to the expected ratio.
- **Recommendation:** supply more input than the minimum needed (e.g. if you need N output frames, supply enough so that after the delay you still get N). Use `input_frames_used` and `output_frames_gen` to track.
- **Fixed block-in / block-out:** size output at least `ceil(src_ratio * input_frames)`. Expect sometimes `output_frames_gen < output_frames` until the filter is warmed up or at end of stream.

---

## end_of_input and streaming

- **During stream:** `end_of_input = 0`. Call `src_process()` repeatedly with new input.
- **At end of stream:** set `end_of_input = 1` on the last call(s) so the converter can **flush** and produce remaining output. Keep calling until `output_frames_gen` is 0 or no more output needed.
- **Continuous real-time (no explicit “end”):** keep `end_of_input = 0`. Use [src_reset()](https://libsndfile.github.io/libsamplerate/api_full.html#src_reset) only when starting a new, unrelated stream (e.g. new file or sample rate change).

---

## Quality and converter type

See [quality.html](https://libsndfile.github.io/libsamplerate/quality.html).


| Converter                   | Use case                                                 |
| --------------------------- | -------------------------------------------------------- |
| **SRC_SINC_BEST_QUALITY**   | Highest quality, most CPU; offline                       |
| **SRC_SINC_MEDIUM_QUALITY** | Good quality, faster; **real-time** (Scyclone uses this) |
| **SRC_SINC_FASTEST**        | Fastest, lower quality                                   |


SINC converters need **state** across calls (in the `SRC_STATE`* from `src_new()`). One state per stream; for real-time, [src_clone()](https://libsndfile.github.io/libsamplerate/api_full.html#src_clone) can snapshot state.

---

## Ratio and numeric precision

[FAQ Q7](https://libsndfile.github.io/libsamplerate/faq.html): `src_ratio` is a `double` (output/input). Double precision is enough for any practical run; ratio drift is sub-sample even over long sessions.

**Q7 vs per-block ratio forcing:** Q7 addresses long-run drift from double precision. Scyclone also uses [FAQ Q6 technique 2](https://libsndfile.github.io/libsamplerate/faq.html) — adjusting `src_ratio` per block to hit exact output counts — which is valid API use but a separate design choice. See [resampling_architecture.md](../../../docs/resampling_architecture.md#design-decision-enforcing-n_out-equals-n) for technique 1 (ring buffer, not adopted) vs technique 2 (chosen) and magnitude bounds.

---

## Other practical points

- **Output levels** ([FAQ Q1](https://libsndfile.github.io/libsamplerate/faq.html)): Upsampling can produce peaks outside [-1, 1]. Normalize before saving to integer formats (e.g. 16-bit WAV).
- **Round-trip** ([FAQ Q3](https://libsndfile.github.io/libsamplerate/faq.html)): 44.1→96→44.1 is **not** bit-identical; anti-alias filter and transients cause small differences.
- **Reset** ([FAQ Q5](https://libsndfile.github.io/libsamplerate/faq.html)): Call `src_reset()` when switching to **new, unrelated** audio (e.g. new file or sample rate); do **not** reset between consecutive blocks of the same stream.
- **Set ratio:** [src_set_ratio()](https://libsndfile.github.io/libsamplerate/api_full.html#src_set_ratio) can force an immediate ratio change (no smooth transition) if needed.

---

## Relation to Scyclone

**Architecture (graph, block contracts, latency):** [resampling_architecture.md](../../../docs/resampling_architecture.md)

- **Implementation:** [ResamplingProcessor.cpp](ResamplingProcessor.cpp), [ResamplingProcessor.h](ResamplingProcessor.h)
- **Converter:** SRC_SINC_MEDIUM_QUALITY; Full API with one `SRC_STATE`* per processor.
- **Upsampler:** `ceil(48000/hostSR × N)` output block; adjusted `srcRatio = N_up/N`.
- **Downsampler:** optional `forcedOutputBlockSize` (host `N`); `srcRatio = N/N_up` (technique 2 — see [architecture doc](../../../docs/resampling_architecture.md#design-decision-enforcing-n_out-equals-n)).
- **Streaming:** `end_of_input = 0` in `processBlock()` (continuous stream). Partial first blocks after reset are expected (transport delay).
- **Latency:** Measured with an impulse test in `prepare()` using streaming `srcRatio`; reported in **input-rate** samples. Fallback uses nominal SINC medium delay in input samples: `46 / min(src_ratio, 1.0)` (see [src_sinc.c](../../../modules/libsamplerate/src/src_sinc.c) lines 456–461: filter half-length widens only for downsampling).
