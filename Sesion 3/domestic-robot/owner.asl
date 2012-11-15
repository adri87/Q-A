!get(beer).  // initial goal

/* Plans */ 

@g 
+!get(beer) : true 
   <- .send(robot, achieve, has(owner,beer)).

@h1 
+has(owner,beer) : true 
   <- !drink(beer).
@h2 
-has(owner,beer) : true 
   <- !get(beer).

// while I have beer, sip   
@d1 
+!drink(beer) : has(owner,beer)
   <- sip(beer);
      !drink(beer).
@d2 
+!drink(beer) : not has(owner,beer)
   <- true.
 
+msg(M)[source(Ag)] : true 
   <- .print("Message from ",Ag,": ",M); 
      -msg(M).
