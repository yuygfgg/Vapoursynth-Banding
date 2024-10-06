# VapourSynth Banding Effect Plugin

This VapourSynth plugin artificially enhances banding in 8-bit video by reducing pixel values to 6-bit and then stretching them back to 8-bit, causing a noticeable banding effect. It is intended for educational purposes to demonstrate how banding occurs in video processing.

## Installation
   ```bash
   meson setup build
   ninja -C build install
   ```

6. The compiled plugin file will be placed in the `build` directory. Copy the resulting shared library (e.g., `banding_plugin.dll` for Windows or `libbanding_plugin.so` for Linux) to your VapourSynth plugin directory.

## Usage


```python
import vapoursynth as vs
core = vs.core

# Load your video
clip = core.ffms2.Source("input_video.mp4")

# Apply the banding effect
clip = core.banding.Filter(clip=clip, enabled=1)

# Output the result
clip.set_output()
```
