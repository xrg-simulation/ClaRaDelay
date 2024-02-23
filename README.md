# ClaRaDelay

[![CI](https://github.com/xrg-simulation/ClaRaDelay/actions/workflows/ci.yml/badge.svg)](https://github.com/xrg-simulation/ClaRaDelay/actions/workflows/ci.yml)

Vectorized Delay block for Modelica

In contrast to the Modelica Delay, the ClaRaDelay vectorizes in time, i.e. you
can specify a vector of previous times at which you want to know the values of a
variable and get a vector with the corresponding values as a result. This is not
only relevant for recurrent networks, but also for convolution integrals of
signals in transmission line models.

The model
[ClaRaDelay.Examples.ExampleClaRaDelay](ClaRaDelay\Examples\ExampleClaRaDelay.mo)
demonstrates the usage.

## C Code Compilation

See [CSource/README.md](./CSource/README.md).

## License

See [CSource/LICENSE](./CSource/LICENSE).
