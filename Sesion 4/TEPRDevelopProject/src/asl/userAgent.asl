// Agent userAgent in project TEPRDevelopProject

/* Initial beliefs and rules */

/* Initial goals */

/* Plans */

+userMsg(Msg) : true 
	<-  Query = 123;
		.send(nluAgent, tell, userMsg(Query, Msg)).
