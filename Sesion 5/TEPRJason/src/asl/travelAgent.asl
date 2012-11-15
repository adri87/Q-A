/* Initial beliefs*/
query_id(1).
//tar(_).

/* Rules */

//turista :- tar(turista).
//preferente :- tar(preferente).
//litera :- tar(litera).

/* Initial goals */
   
/* Plans */		
//@storefare
//+fare(F) : true
//	<-	+fare(F).
	
@storejourney
+journey(From, To, Departure, Arrival, fare(FName, FPrice))[query(Query)]
	: 	true
	<- 	.print(journey(From, To, Departure, Arrival, fare(FName, FPrice)));
		+there_travel(yes);
		+journey(From, To, Departure, Arrival, fare(FName, FPrice)).
		
@find	
+!find_travel(From,To,Day,Month,Year) 
	:	true
	<-	?query_id(N);
		Id = N + 1;
		-+quer_id(Id);
		findTravel(Id, From, To, Day, Month, Year);
		!min.

@search
//+!min :	turista & ok
//	<- 	.send(userAgent, achieve, wait);
//		.findall(alljourney(fare(FPrice, turista), Arrival, Departure, To, From), 
//			journey(From, To, Departure, Arrival, fare(turista, FPrice)), L);
//		.min(L, alljourney(fare(FPriceMin, FN), Arri, Depart, T, F));
//		.send(userAgent, tell, available(yes));
//		.send(userAgent, achieve, show(journey(F, T, Depart, Arri, fare(FN, FPriceMin))));
//		-fare(_);
//		.abolish(journey(_,_,_,_,_)).
		
//+!min : preferente & ok
//	<- 	.send(userAgent, achieve, wait);
//		.findall(alljourney(fare(FPrice, preferente), Arrival, Departure, To, From), 
//			journey(From, To, Departure, Arrival, fare(preferente, FPrice)), L);
//		.min(L, alljourney(fare(FPriceMin, FN), Arri, Depart, T, F));
//		.send(userAgent, tell, available(yes));
//		.send(userAgent, achieve, show(journey(F, T, Depart, Arri, fare(FN, FPriceMin))));
//		-fare(_);
//		.abolish(journey(_,_,_,_,_)).
		
//+!min : litera & ok
//	<- 	.send(userAgent, achieve, wait);
//		.findall(alljourney(fare(FPrice, litera), Arrival, Departure, To, From), 
//			journey(From, To, Departure, Arrival, fare(litera, FPrice)), L);
//		.min(L, alljourney(fare(FPriceMin, FN), Arri, Depart, T, F));
//		.send(userAgent, tell, available(yes));
//		.send(userAgent, achieve, show(journey(F, T, Depart, Arri, fare(FN, FPriceMin))));
//		-fare(_);
//		.abolish(journey(_,_,_,_,_)).

+!min : there_travel(yes)
	<- 	.findall(alljourney(fare(FPrice, FName), Arrival, Departure, To, From), 
			journey(From, To, Departure, Arrival, fare(FName, FPrice)), L);
		.min(L, alljourney(fare(FPriceMin, FN), Arri, Depart, T, F));
		.send(userAgent, tell, available(yes));
		.send(userAgent, achieve, show(journey(F, T, Depart, Arri, fare(FN, FPriceMin))));
		.send(userAgent, tell, available(no));
		-there_travel(_);
		.abolish(journey(_,_,_,_,_)).
		
+!min : true
	<-	.send(userAgent, achieve, show(journey(_,_,_,_,_)));
		.abolish(journey(_,_,_,_,_)).
