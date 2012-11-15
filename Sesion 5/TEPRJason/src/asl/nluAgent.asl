/* Initial beliefs */
//fare(_).

/* Rules */

month(1) :- month(enero).
month(2) :- month(febrero).
month(3) :- month(marzo).
month(4) :- month(abril).
month(5) :- month(mayo). 
month(6) :- month(junio).
month(7) :- month(julio). 
month(8) :- month(agosto). 
month(9) :- month(septiembre). 
month(10) :- month(octubre). 
month(11) :- month(noviembre). 
month(12) :- month(diciembre).

//there_fare :- fare(turista) | fare(preferente) | fare(litera). 
	
/* Initial goals */

/* Plans */
+people(P) : true
	<-	+people(P).
	
+location_from(From) : true
	<-	+location_from(From).
		
+location_to(To) : true
	<- 	+location_to(To).

+day(D) : true
	<- 	+day(D).
		
+month(M) : true
	<- 	+month(M).
		
+year(Y) : true
	<- 	+year(Y).
	
+fare(F) : true
	<-	+fare(F).
	
@nlu
+!nlu(Msg) : true 
	<- 	sendNLU(Msg);
		+msg(Msg);
		!go.

@checking
+!go : not location_from(_)
	<-	.send(userAgent, achieve, ask_from).

+!go : not location_to(_)
	<-	.send(userAgent, achieve, ask_to).
		
+!go : not people(_)
	<-	.send(userAgent, achieve, ask_people).
		
+!go : not day(_) | not month(_) | not year(_)
	<-	-day(_);
		-month(_);
		-year(_);
		.send(userAgent, achieve, ask_date).

@running
//+!go : there_fare
//	<-	!assign_month;
//		?fare(F);
//		.send(travelAgent, tell, tar(F));
//		?people(P);
//		?location_from(From);
//		?location_to(To);
//		?day(D);
//		?month(M);
//		?year(Y);
//		.wait(2000);
//		.send(travelAgent, achieve, find_travel(From, To, D, M, Y));
//		-fare(_);
//		-people(_); 
//		-location_from(_);
//		-location_to(_);
//		-day(_);
//		-month(_);
//		-year(_);
//		-msg(_).

+!go : true
	<-	!assign_month;
		?people(P);
		?location_from(From);
		?location_to(To);
		?day(D);
		?month(M);
		?year(Y);
		.wait(2000);
		.send(travelAgent, achieve, find_travel(From, To, D, M, Y));
		-fare(_);
		-people(_); 
		-location_from(_);
		-location_to(_);
		-day(_);
		-month(_);
		-year(_);
		-msg(_).
		
@numberMonth
+!assign_month : month(1)
	<-	+month(1).
	
+!assign_month : month(2)
	<-	+month(2).

+!assign_month : month(3)
	<-	+month(3).
	
+!assign_month : month(4)
	<-	+month(4).

+!assign_month : month(5)
	<-	+month(5).
	
+!assign_month : month(6)
	<-	+month(6).
	
+!assign_month : month(7)
	<-	+month(7).
	
+!assign_month : month(8)
	<-	+month(8).
	
+!assign_month : month(9)
	<-	+month(9).
	
+!assign_month : month(10)
	<-	+month(10).
	
+!assign_month : month(11)
	<-	+month(11).
	
+!assign_month : month(12)
	<-	+month(12).
