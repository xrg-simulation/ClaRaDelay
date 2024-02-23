# ClaRaDelay

Vectorized Delay block for Modelica.

In contrast to the Modelica Delay, the ClaRaDelay vectorizes in time, i.e. you
can specify a vector of previous times at which you want to know the values of a
variable and get a vector with the corresponding values as a result. This is not
only relevant for recurrent networks, but also for convolution integrals of
signals in transmission line models.

## Compilation

### Unix

```bash
cmake -S . -B build
cd build
cmake --build . --target install
```

### Visual Studio

```cmd
cmake -S . -B build_vs -G "Visual Studio 16 2019" # or "Visual Studio 17 2022"
cd build_vs
cmake --build . --target install
```

### MSYS2

```bash
cmake -S . -B build_msys -G "MSYS Makefiles"
cd build_msys
make -j -Oline install
```
