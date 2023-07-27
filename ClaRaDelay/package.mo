within ;
package ClaRaDelay
  annotation (
    uses(Modelica(version="4.0.0")),
    Documentation(info="<html>
<p><span style=\"font-family: Helvetica; background-color: #ffffff;\">Vectorized Delay block for Modelica</span></p>
<p><span style=\"font-family: Helvetica;\">In contrast to the Modelica Delay, the ClaRaDelay vectorizes in time, i.e. you can specify a vector of previous times at which you want to know the values of a variable and get a vector with the corresponding values as a result. This is not only relevant for recurrent networks, but also for convolution integrals of signals in transmission line models.</span></p>
<p><br><span style=\"font-family: Helvetica;\">The model <a href=\"modelica://ClaRaDelay.Examples.ExampleClaRaDelay\">ClaRaDelay.Examples.ExampleClaRaDelay</a> demonstrates the usage.</span></p>
</html>"),
    Icon(graphics={Bitmap(extent={{-100,-100},{100,100}}, fileName="modelica://ClaRaDelay/Resources/Images/Packages/ClaRa.png")}),
  version="0.2.0");
end ClaRaDelay;
