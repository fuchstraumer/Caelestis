## Foundation

Foundation is the baseline of the Caelestis project, and is where common code that can be used
by all plugins can be found. The intention of this is to improve some of the pains of DLL-ificiation,
by providing a stable ABI of functionality and objects that all plugins can use and exchange between
each other. 

This will probably eventually be further tied to the concept of "Capstone", what I plan to use
as the final layer binding all of the plugins together and providing some important frameworks
and functionality for doing so (as part of building a final application).

All said, the point is to allow for a safe and stable ABI for plugins still - but to make sure we're
not forcing ourselves to pointlessly deal with a really painful C-style ABI (compromising dev time further,
and potentially at the cost of perf too). Adding to this will probably happen frequently during the
beginning of the project but that's okay while I get it up and running and built out. Fun developing
this project is just as important to me as making it good!
