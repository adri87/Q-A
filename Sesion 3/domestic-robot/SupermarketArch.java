import jason.asSyntax.*;
import jason.asSemantics.*;
import jason.architecture.*;
import java.util.*;

public class SupermarketArch extends AgArch {

  @Override
  public void checkMail() {

    // calls the default implementation to move all
    // messages to the circumstance's mailbox.
    super.checkMail();

    // gets an iterator for the circumstance's mailbox
    // and removes messages from owner
    Iterator im = getTS().getC().getMailBox().iterator();
    while (im.hasNext()) {
      Message m = (Message) im.next();
      if (m.getSender().equals("owner")) {
        im.remove();
        
        // sends a message to owner to inform that
        // his/her message was ignored
        Message r = new Message(
          "tell",
          getAgName(),
          m.getSender(),
          "msg(\"You are not allowed to ask me for anything, only your robot can do that!\")");
        try {
          sendMsg(r);
        } catch (Exception e) {}       
      }
    }   
  }
}
