Dependency
----------
> #### Linux (ubuntu 12.04 x64 tested)
> ```
> apt-get install build-essential
> apt-get install gcc-multilib
> apt-get install cmake
> apt-get install libstdc++6-4.4-dev gcc-4.4-multilib g++-4.4-multilib
> apt-get install g++-multilib
> apt-get install ia32-libs
> ```
> #### Windows
> ```
> all dependencies are includes in the repository and will be build
> ```

Build (command line)
--------------------
> ```
> git clone https://gitlab.q3df.org/megadaddeln/recordsystem2.git
> cd recordsystem2
> git submodule update --init
> ```
> 
> > #### Windows (in vs2010 cmd shell)
> > ```
> > create_projects.bat
> > build_projects.bat <Debug|Release>
> > ```
> > #### Linux (bash shell)
> > ```
> > ./create_projects.sh
> > ./build_projects.sh <Debug|Release>
> > ```

Build (windows vs2010 / vs2012)
-------------------------------
> ```
> git clone https://gitlab.q3df.org/megadaddeln/recordsystem2.git
> cd recordsystem2
> git submodule update --init
> create_projects.bat
> ```
> now you can open the generated recordsystem.sln solution!> 