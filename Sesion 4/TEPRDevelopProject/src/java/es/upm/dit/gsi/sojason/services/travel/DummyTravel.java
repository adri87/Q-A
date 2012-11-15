/**
 * 
 */
package es.upm.dit.gsi.sojason.services.travel;

import jason.asSyntax.Literal;

import java.util.Collection;

import es.upm.dit.gsi.sojason.services.WebServiceConnector;

/**
 *
 * Project: TEPRDevelopProject
 * Package: es.upm.dit.gsi.sojason.services.travel
 * Class: DummyTravel
 *
 * @author Miguel Coronado (miguelcb@dit.upm.es)
 * @version Mar 7, 2012
 *
 */
public class DummyTravel implements WebServiceConnector {

	/**
	 * Parameters expected
	 * <ul>
	 * 	<li>query</li>
	 * 	<li>from</li>
	 * 	<li>to</li>
	 *  <li>day</li>
	 *  <li>Month</li>
	 *  <li>Year</li>
	 * </ul> 
	 * 
	 * Recommended belief format
	 * <ul>
	 * 	<li>journey(madrid,barcelona,time(18,20),time(19,18),fare(turista,33.2))[query(38009)]</li>
	 *  <li>journey(madrid,barcelona,time(18,20),time(19,18),fare(bussiness,53.8))[query(38009)]</li>
	 * <ul>
	 * 
	 */
	public Collection<Literal> call(String... params) {
		// TODO Auto-generated method stub
		return null;
	}

	public boolean validateParams(String... params) {
		// TODO Auto-generated method stub
		return false;
	}

}
