![Ptahka logo](docs/res/logo.svg)
# **Ptah**ka
## **The *godlike* web framework** using almighty technologies to face modern challenges
[![HitCount](http://hits.dwyl.io/phtdacosta/ptahka.svg)](http://hits.dwyl.io/phtdacosta/ptahka)
[![Build status](https://ci.appveyor.com/api/projects/status/byeaerrdevtkwtfo/branch/master?svg=true)](https://ci.appveyor.com/project/phtdacosta/ptahka/branch/master) [![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/phtdacosta/ptahka.svg)](http://isitmaintained.com/project/phtdacosta/ptahka "Average time to resolve an issue")
[![Percentage of issues still open](http://isitmaintained.com/badge/open/phtdacosta/ptahka.svg)](http://isitmaintained.com/project/phtdacosta/ptahka "Percentage of issues still open")
[![Ptahka integration status](https://img.shields.io/badge/Windows-supported-brightgreen.svg)](https://github.com/phtdacosta/ptahka "Ptahka integration status")
[![Ptahka integration status](https://img.shields.io/badge/Unix-unsupported-red.svg)](https://github.com/phtdacosta/ptahka "Ptahka integration status")

# `main` method in **3 lines**
```c
// Group where all routes are added
table_t *table = inittable();

// Create customizable HTTP routes 
createroute("GET",       // HTTP method
            "/home",     // Resource route
            &Homepage,   // Function pointer
            table);      // Route group

// Fires the server engine forever
initservice(&parserequest, table, &executeroute, NULL);
``` 

# **Brutally fast**, period.
**Ptah**ka is brutally fast. **Built from scratch** using just plain C language and specially written functions on top of the most performant Windows NT4 networking API, you can truly expect the ultimate performance available on any machine.
### Extra technical information for the geeks
High modularity in core aspects of a project often means easier tweaks, higher adaptability and consequently, more performance.

One of the two parts that composes **Ptah**ka is **[Gobio](https://github.com/phtdacosta/gobio)**, powering all low-level networking stuff, input and output strategy (further information [here](https://tangentsoft.net/wskfaq/articles/io-strategies.html)), multi-threading and the general socket behavior. Gobio **benchmarks shows 3.87x more speed than NodeJS** which relies on the same pillars when running on Windows, ouch!

The second part is **[Tinfin](https://github.com/phtdacosta/tinfin)** which contains useful specialized functions, for handling protocols of a higher level more intuitively.
# Focused on **productivity**
The design patterns were inspired by very popular frameworks like *Flask*, *Falcon* (both written in Python) and *NodeJS*. Like those, **Ptah**ka achitecture allows developers to rapidly prototype and build server solutions, from **microservices** to **full-featured API's**.
# Born **modular** and **scalable**
It was born modular since the whole framework **is just two other smaller projects tied up with some magic dust**. Gobio brings high-performance socket support and Tinfin specialized protocol handling. Feel free to modify both or even replace them.

Scalability is achieved thru the Makefile script available within the project. It is **enough to automatically compiles any other source files (*.c) that small or medium projects could ever need**, and may be even useful for big projects as well.
# Easily **extensible** and widely **compatible**
Written in pure and plain C99, **Ptah**ka **API can be easily extended by any other language that extend C language**, like *C++, D, Go, Rust, Python, Lua, NodeJS* and so on, making the project reliable enough to become a solution anywhere.

The **stability** of the Winsock 2 API (which was first implemented for Windows 95) **makes it perfect to be used on any Windows NT4 compatible (legacy ones included) project**.
## Download, build and run
```
> git clone https://github.com/phtdacosta/ptahka.git
> cd ptahka
> make -B all
> main
```
## Documentation
The documentation is going to be written gradually, but enough information regarding the basic usage of the framework is already available in the `main` file included.
## Community and purpose
**Ptah**ka was created with community in mind. Giving developers a reliable option to create powerful web applications using native and extremely performant technologies, yet focused in productivity, simplicity and modularity, empowers the ones that will shape the digital future without forgetting stable and well-tested tools from years ago. **Faster web applications means efficient energy use on data warehouses which leads to less overall energy consumption. Performance is sustainability.** Think about that. (I will write something about this in the future!)

[![Join the chat at https://gitter.im/ptahka/Lobby](https://badges.gitter.im/ptahka/Lobby.svg)](https://gitter.im/ptahka/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
## License
Feel free to do whatever you want with the code. **Ptah**ka is open source and [**MIT**](https://github.com/phtdacosta/ptahka/blob/master/LICENSE) licensed.
## Support
The whole project is maintained for just one person, so just-in-time support is almost impossible. But critical bugs and significant issues are going to be patched as soon as possible.
## Milestones
* Detailed documentation with advanced usage examples
* Make **Ptah**ka less bug prone as possible without making it slower
* Further micro-optimizations and even broader machine support