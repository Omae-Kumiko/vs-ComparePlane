# vs-ComparePlane
Optimize for several algorithms for compare images.

## Usage
```python
core.complane.psnr(clip clip1, clip clip2, int opt, int cache)
```
The both input clips must be of GRAY colorFamily and their format must be exactly same. Both 8-16 int and float sample type are allowed.
The res will be reserved into the frame props of output.
1. Args:
  - ```clip1```: The clip you want to complete, the result have the same frame data and props (except the added by filter) as this clip.
  - ```clip2```: The other clip.
  - ```opt```: Optimize level:
    - 0: auto detect(default)
    - 1: sse2
    - 2: avx2
  -  ```cache```: Whether the output should be cache or not. If you want to access them later, it's recommand to enable cache
    - 0: disable
    - 1: enable(default)  
