import java.io.IOException;
import java.util.ArrayList;

/**
 * @author Jaroslav Klaus
 */
public class Receiver extends Thread {
	/**
	 * Connection
	 */
	private Connection udp;

	/**
	 * Buffer for received messages
	 */
	private ArrayList<Message> buffer;

	/******************************************************************************************************************/

	/**
	 * Constructor for a thread that receives messages
	 */
	public Receiver(Connection udp) {
		this.udp = udp;
		buffer = new ArrayList<Message>();
		this.start();
	}

	/**
	 * Run method of the thread. Receives message from socket and adds it to the buffer
	 */
	@Override
	public void run() {
		while (true) {
			try {
				Message m = udp.receiveMessage();
				buffer.add(m);
			} catch (IOException e) {
				System.out.println("UDP receiver error");
			}
		}
	}

	/**
	 * Getter for a message from a buffer
	 *
	 * @return a message from a buffer or null
	 */
	synchronized public Message getMessageFromBuffer() {
		if (buffer.size() > 0) {
			return buffer.remove(0);
		}
		return null;
	}
}
