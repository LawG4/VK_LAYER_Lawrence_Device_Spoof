# VK_LAYER_Lawrence_Device_Spoof

This is my educational project where I experiment with attempting to build my own layer. The point of this is to explore the Vulkan loader and to get a deeper understanding of its internals. I suppose you could see this as a precursor to Vulkan driver development.

**Thoughts on this project?** - I learned a lot about function pointers and how call chains work, and a lot about Vulkan instance creation. However I am not very confident in how robust the layer is, especially on multiple platforms.



Build instructions.

```bash
# Pull the repo 
git clone https://github.com/LawG4/VK_LAYER_Lawrence_Device_Spoof.git
cd VK_LAYER_Lawrence_Device_Spoof

# Configure the build
mkdir build
cd build
cmake ..

# Build
cmake --build .
```

From what I've seen you'll have to use Unix make files on linux. It seems that ninja actually hates ASM for the Vulkan loader.  

