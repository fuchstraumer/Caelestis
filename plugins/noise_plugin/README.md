# CanisNoise

Based on my old CUDA_Noise project, CanisNoise aims to recreate the successes of that project but using Vulkan instead. So no Vendor locking, and because I'm not entirely sold on CUDA as an API (to say the least).

It's WIP as hell right now and being torn down nearly completely but the end goal is:

1. Have a backend that can be linked into other projects as a DLL, allowing for noise generation and handling the dependencies/chaining of dispatches between modules (plus some memory work, too!)
2. Have a frontend demo using ImGui + some nodegraph magic, so that users can experiment with the right layout/settings for their noise chains. Ideally, I'll let these be serialized to/from JSON so that a setup created in the GUI can be used with the library ^^

This is all HEAVILY tied to my work with DiamondDogs and VulpesSceneKit: this will be eventually used to power procedural terrain generation in my projects. Development might stop/start sporadically, currently as I prepare VPSK and DiamondDogs for terrain stuff after several months away.
