# Plugins

Plugins are what define the real functionality of an application: more advanced functionality can be composed and enabled by loading more and more plugins.

I'll save the real documentation for the individual plugins, but adding further plugins can be done as follows:
- Create a new directory, and add a `CMakeLists.txt` to it
- Call the function `ADD_PLUGIN`, passing the name of the plugin and your source files
- After this, simply specify the include directories you wish to use and the unique libraries you wish to link to
- Make sure to define a unique API ID as well, along with being able to return a mostly-complete `Plugin_API*` from your `GetPluginAPI()` function. This should be the only exported function, ideally!
