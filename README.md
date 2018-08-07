# Caelestis

Caelestis is my attempt at a Vulkan rendering engine - but using a highly plugin-based architecture. Each individual folder in `plugins` is an almost entirely independent "module" of code. I have attempted to keep these modules clear and apparent in regards to their responsibilities, and will be adding `README`s to them as I get the time. 

So far, it features:
- a base utility layer of common application building blocks and utility functions
- a rendering context layer, which is highly configurable and works on multiple platforms (using Vulkan!)
- a resource context layer, which helps manage Vulkan resources in a simpler manner
- it's all cross-platform! It's been tested so far on Mac OSX, Win10, and Ubuntu 17.10. Mac requires `brew`, primarily to install Boost so we can still get `variant` and `filesystem` when using XCode's dated version of clang (Apple pls)

### Plugin System?

I have attempted to use a plugin system akin to the one espoused by our-machinery [here](http://ourmachinery.com/post/little-machines-working-together-part-1/) (and in the corresponding second part, along with [this]() extension on hot-reloading). I currently don't support hot-reloading, however, as my plugins don't hold enough state to make that worthy. Additionally, (as you will find out) I haven't quite yet got CMake to install the compiled plugin shared libs robustly. So, it's a bit difficult.

The good news is that debugging should mostly work: potential errors with the PDB should be fixed on Windows, as sometimes the manually-specified path in the compiled binary can break when one moves the binary itself around. The solution was provided by the wonderful [cr](https://github.com/fungos/cr), a great example of a single-header plugin system (though with a different approach to handling hot reloading: it's worth a look, along with the article the author wrote [over here](https://fungos.github.io/blog/2017/11/20/cr.h-a-simple-c-hot-reload-header-only-library/)!)

The second bit of good news is that the plugin system has made it easy to figure out what does what: `application_context` is for your low-level facilities you'd expect to be the groundwork of any application. `renderer_context` sets up the Vulkan Instance, chooses a singular physical device (but supports multiple! just haven't got to it yet), and sets up the Logical Device too. It also sets up the primary swapchain you'll use for presenting, though I want to add "virtual" swapchains as secondary rendertargets and such. One can also register to receive input events, and swapchain recreation events.

`resource_context` provides a single managed interface for the creation of Vulkan resources like samplers, buffers, and images. It also exposes an asynchronous asset loading system, and transfers of data from staging buffers to the GPU resources are performed asynchronously as well. It also sits on top of the `vpr_alloc` plugin, and manages a single `vpr::Allocator` instance used to more efficiently allocate and manage `VkDeviceMemory` objects (bind to subregions, split into size and type pools to reduce fragmentation).

I'll hopefully eventually get around to documenting all of these more, but this project is currently being developed in parallel with my work for my job: it often mirrors work done there, and you might see catch-up commits. Some content is removed, of course, to preserve confidientality. 

### Building/Installing

For now, you can attempt to download and build + run the examples as you see fit. It should work natively - though the rather large quantity of submodules in `third_party` is a hard requirement - so a git-clone works better than just downloading the `.zip`.

The only further steps will be copying two things into the binary directory of the test executables:
- The DLLs of the compiled plugins and dependencies: these will be installed to `${CMAKE_INSTALL_PREFIX}/bin`. I personally use a folder called `ext` in the root directory of this project for my CMake install directory
- Compiled shader code from the plugin test directories should be installed, as well.

I'd like to automate this step but I'm a bit stuck as to how: CMake doesn't seem to have any easy way to do it that doesn't get messy and gross fast.
