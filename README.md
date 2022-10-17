# ClaRaDelay
Vectorized Delay block for Modelica

Im Gegensatz zum Modelica-Delay vektorisiert das ClaRa-Delay in der Zeit, dass heißt man kann einen Vektor von vorherigen Zeitpunkten, an denen man die Werte einer Variablen wissen will, vorgeben und erhält einen Vektor mit den entsprechenden Werten als Ergebnis. Dies ist nicht nur bei rekurrenten Netzwerken relevant , sondern zB auch für Faltungsintegrale von Signalen bei TransmissionLine Modellen.
