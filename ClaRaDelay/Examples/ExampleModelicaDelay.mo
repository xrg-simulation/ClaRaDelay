within ClaRaDelay.Examples;
model ExampleModelicaDelay

  parameter Real samplePeriod = 0.1;
  parameter Integer nHistoricElements = 5;

  Real signal = sin(2*Modelica.Constants.pi*time);
  Real[nHistoricElements] delayedSignals;

equation

  for t in 1:nHistoricElements loop
    delayedSignals[t] = delay(signal, samplePeriod*(t - 1));
  end for;

  annotation (
    Icon(coordinateSystem(preserveAspectRatio=false), graphics={Bitmap(extent={{-100,-100},{100,100}}, fileName="modelica://ClaRaDelay/Resources/Images/Packages/ExecutableExample_b80.png")}),
    Diagram(coordinateSystem(preserveAspectRatio=false)),
    Documentation(info="<html>
<p>This example serves as reference how to delay a signal multiple times with the Modelica delay.</p>
</html>"),
  experiment(StartTime = 0, StopTime = 1, Tolerance = 1e-6, Interval = 0.002));
end ExampleModelicaDelay;
