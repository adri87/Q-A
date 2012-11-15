// Agent sample in project EjercicioFiltradoTime

/* Initial beliefs and rules */

is_before(HH,MM, Hh,Mm) :- true.
is_after(HH,MM, Hh,Mm) :- true.

starts(after, time(15,40)).
ends(before, time(18,15)).

/* Initial goals */

!start.

/* Plans */

+!start : true 
	<- 	+event("Workshop qa", time(14,00), time(16,00))[topic(qa)];
		+event("Coffe break", time(16,00), time(16,30))[topic(other)];
		+event("Workshop agents", time(16,30), time(18,00))[topic(agents)];
		
		+event("Conference qa", time(14,30), time(15,20))[topic(qa)];
		+event("Conference agents", time(15,30), time(16,20))[topic(agents)];
		+event("Conference Jason", time(16,30), time(17,20))[topic(agents)];
		
		.at("now +2 second", "+!show(results)").
		
/* Filter events when created */
+event(Msg, time(StartH, StartM), End)
	:	starts(after, time(HH, MM) & is_after(HH, MM, StartH, StartM)
	<- 	.print("Remove: ", event(Msg, time(StartH, StartM), End));
		.event(Msg, time(StartH, StartM), End).

+event(Msg, Start, time(EndH, EndM))
	:	starts(before, time(HH, MM) & is_before(HH, MM, StartH, StartM)
	<- 	.print("Remove: ", event(Msg, Start, time(EndH, EndM)));
		.event(Msg, Start, time(EndH, EndM)).

+event(Msg, Star, End)
	:	true
	<-	.print("Keep", event(Msg, Star, End)).
		
/* show results */
@show
+!show(results)	: true
	<-	.findall(Msg, event(Msg,_,_), List);
		List \= [];
		.findall("Puedes ir a :");
		.print(List).