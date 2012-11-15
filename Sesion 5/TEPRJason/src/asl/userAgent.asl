/* Initial beliefs*/
available(no).
from.
to.
people.
date.

/* Rules */

/* Initial goals */

/* Plans*/
@msg
+user_msg(Msg): not from
	<- 	.concat("desde ", Msg, X);
		+from;
		.send(nluAgent, achieve, nlu(X)).

+user_msg(Msg): not people
	<- 	.concat(Msg, " personas", X);
		+people;
		.send(nluAgent, achieve, nlu(X)).
		
+user_msg(Msg): not to
	<- 	.concat("a ", Msg, X);
		+to;
		.send(nluAgent, achieve, nlu(X)).
		
+user_msg(Msg): not date
	<- 	.concat("el ", Msg, X);
		+date;
		.send(nluAgent, achieve, nlu(X)).
		
+user_msg(Msg) : true 
	<-	.send(nluAgent, achieve, nlu(Msg)).
	
@tellUserAf
+!show(journey(From, To, Departure, Arrival, fare(FName, FPrice)))
	: available(yes)
	<-	sendUser(journey(From, To, Departure, Arrival, fare(FName, FPrice)));
		+available(no).

@tellUserNeg
+!show(journey(From, To, Departure, Arrival, fare(FName, FPrice)))
	: available(no)
	<-	sendUser("Sin billetes para los parametros seleccionados").

@requests	
+!ask_people : true
	<- 	-people;
		sendUser("Numero de personas").
		
+!ask_from : true
	<- 	-from;
		sendUser("Ciudad de salida").
	
+!ask_to : true
	<- 	-to;
		sendUser("Ciudad de destino").
	
+!ask_date : true
	<- 	-date;
		sendUser("Fecha del viaje").	
