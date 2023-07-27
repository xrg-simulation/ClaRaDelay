# ClaRaDelay
Vectorized Delay block for Modelica

In contrast to the Modelica Delay, the ClaRaDelay vectorizes in time, i.e. you can specify a vector of previous times at which you want to know the values of a variable and get a vector with the corresponding values as a result. This is not only relevant for recurrent networks, but also for convolution integrals of signals in transmission line models.
