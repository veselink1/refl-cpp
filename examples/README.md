# refl-cpp
## Examples

- **Implement a simple serialization system** - [example-serialization.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-serialization.cpp)
    Shows how to implement a very efficient and generic serialization procedure from scratch

- **Implement a generic builder class factory** - [example-builders.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-builders.cpp)
    Shows how to utilize refl-cpp proxy classes to define a generic `builder<T>` class, which implements the builder pattern

- **Simple SQL database abstraction** - [example-dao.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-dao.cpp)
    Shows how to implement a basic ORM system which can generate SQL statements at compile-time from model classes using custom properties

- **Iterate base classes with `bases<>`** - [example-inheritance.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-inheritance.cpp)
    Shows to use the built-in `bases<>` attribute to iterate over a type's base classes

- **Access reflection information at runtime** - [example-custom-rtti.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-custom-rtti.cpp)
    Shows how to implement a basic runtime abstraction over refl-cpp which provides access to the reflection metadata at runtime via custom metadata objects

- **Type factories with proxies** - [example-proxy.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-proxy.cpp)
    Shows to implement a basic type factory that exposes target type fields as functions

- **XML-based GUI with strongly-types properties** - [example-binding.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-binding.cpp)
    Shows how to implement a system for reading XML resources describing a UI for an imaginary GUI system with refl-cpp
