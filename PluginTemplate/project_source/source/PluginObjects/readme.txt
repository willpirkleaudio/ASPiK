Use the PluginObjects folder to store your local plugin project files. This
includes the fxobjects.h, fxobjects.cpp, and filters.h files as documented in
the FX book as well as the ASPiK documentation.

You can download these files from: https://www.willpirkle.com/Downloads/fxobjects.zip

Then, store them in your project's PluginObjects folder, and manually add them to the
Xcode and/or Visual Studio projects in the same manner that you normally would
add new (existing) project files. 

Please do NOT add these files to the ASPiK SDK as there are many users who do
not want them, and Git merges will become confusing. Note that you can also
use RackAFX7 to generate ASPiK projects with these files already included
since they are part of RackAFX7 projects as well, but only as 3rd party
supplies (there is NO RackAFX code in the fxobjects.h and .cpp files).

- Will Pirkle
