package es.upm.dit.gsi.sojason;

import jason.asSyntax.Literal;
import jason.asSyntax.Term;

import java.io.IOException;
import java.util.Collection;
import java.util.logging.Logger;

import es.upm.dit.gsi.jason.utils.CollectionUtils;
import es.upm.dit.gsi.sojason.services.WebServiceConnector;
import es.upm.dit.gsi.sojason.services.nlu.DummyNlu;
import es.upm.dit.gsi.sojason.services.travel.DummyTravel;

/**
 * 
 *
 * Project: Web40
 * Package: es.upm.dit.gsi.qa
 * Class: Web40Model
 *
 * @author Miguel Coronado (miguelcb@dit.upm.es)
 * @version Feb 29, 2012
 *
 */
public class QaSoModel extends SOModel{

	/** The Natural Language Service Connector */
	private WebServiceConnector nluConnector;
	
	/** The RENFE Scraper */
	private WebServiceConnector travelService;
	
	/** the Logger */
	private Logger logger = Logger.getLogger("SOJason." + QaSoModel.class.getName());
	
	/** Constructor 
	 * @throws IOException */
	public QaSoModel () throws IOException {
		super();
		this.nluConnector = new DummyNlu();
		this.travelService = new DummyTravel();
		
		initialData();
	}
	
	/** Initail beliefs */
	protected void initialData() {
		Literal literal = Literal.parseLiteral("userMsg(\"I want to travel from Madrid to Cuenca\")");
		this.setDataInbox("userAgent", literal);
	}
	
	/**
	 * This calls the NLU service
	 * 
	 * Internally this modifies the model so it reports the agent
	 *  
	 * @param agName 	the name of the agent that will be reported with the 
	 * 					results of the call.
	 * @param terms		The parameters
	 * @return			
	 */
	public boolean sendNlu (String agName, Collection<Term> params) {

		logger.info("Entering sendNLU...");
		try{
			String[] strParams = CollectionUtils.toStringArray(params);
			Collection<Literal> serviceData = nluConnector.call(strParams);
			if(serviceData == null){ return false;	}
			
			// put data into mailbox
			this.setDataInbox(agName, serviceData);
		} 
		catch (Exception e){
			logger.info("Could not complete action sendNLU:" + e.getMessage());
			return false;	
		}
		
		logger.info("NLU call completed successfully");
		return true;
	}
	
	/**
	 * 
	 * @param agName
	 * @param terms
	 * @return
	 */
	public boolean findTravel (String agName, Collection<Term> params) {
		
		logger.info("Entering sendfindTravel...");
		try{
			String[] strParams = CollectionUtils.toStringArray(params);
			Collection<Literal> serviceData = travelService.call(strParams);
			if(serviceData == null){ return false; }
			
			// put data into mailbox
			this.setDataInbox(agName, serviceData);
		} 
		catch (Exception e){
			logger.info("Could not complete action findTravel:" + e.getMessage());
			return false; 
		}
		logger.info("Find Travel call completed successfully");
		return true;
		
	}
	
}
