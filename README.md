# serve.h

Simple Solution for serving a HTTP server in C.

Just copy `serve.h` into your project and include it. And only in one of your translation units, define `SERVE_IMPLEMENTATION` like so:
```c
// code

#define SERVE_IMPLEMENTATION
#include "serve.h"

// code
```
