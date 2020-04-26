# FastSignals 
ATTENTION: it's been forked from https://github.com/ispringteam/FastSignals in order to make it work with C++11

Yet another C++ signals and slots library

* Works as drop-in replacement for Boost.Signals2 with the same API
    * Has better performance and more compact binary code
* Thread-safe in most operations, including concurrent connects/disconnects/emits
* Implemented with compact, pure ~~C++17~~ C++11 code

[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)

## Documentation

* [Why FastSignals?](docs/why-fastsignals.md)
* [Simple Examples](docs/simple-examples.md)
* [Migration from Boost.Signals2](docs/migration-from-boost-signals2.md)
