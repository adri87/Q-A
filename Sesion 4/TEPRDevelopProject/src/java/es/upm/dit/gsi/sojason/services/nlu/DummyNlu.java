/**
 * 
 */
package es.upm.dit.gsi.sojason.services.nlu;

import jason.asSyntax.Literal;

import java.util.Collection;
import java.util.LinkedList;

import es.upm.dit.gsi.sojason.services.WebServiceConnector;

/**
 *
 * Project: TEPRDevelopProject
 * Package: es.upm.dit.gsi.sojason.services.nlu
 * Class: DummyNlu
 *
 * @author Miguel Coronado (miguelcb@dit.upm.es)
 * @version Mar 7, 2012
 *
 */
public class DummyNlu implements WebServiceConnector {

	public Collection<Literal> call(String... params) {
		
		String queryid = params[0];	// first param is the queryid
					// second param is the message. Discard in dummy
		
		Collection<Literal> res = new LinkedList<Literal>();
		res.add(Literal.parseLiteral("cityfrom(madrid)[query(" + queryid + ")]"));
		res.add(Literal.parseLiteral("cityto(barcelona)[query(" + queryid + ")]"));
		res.add(Literal.parseLiteral("ticket(bussiness)[query(" + queryid + ")]"));
		res.add(Literal.parseLiteral("travel(train)[query(" + queryid + ")]"));
		res.add(Literal.parseLiteral("departuretime(10,00)[query(" + queryid + ")]"));
		res.add(Literal.parseLiteral("departureday(8,5,2012)[query(" + queryid + ")]"));
		
		return res;
	}

	public boolean validateParams(String... params) {
		if(params.length != 2){
			return false; // two params expected
		}
		return true;
	}

}
