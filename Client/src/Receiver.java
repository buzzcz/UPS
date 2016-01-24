import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;
import java.util.concurrent.Semaphore;

/**
 * @author Jaroslav Klaus
 */
public class Receiver extends Thread {
	/**
	 * Connection
	 */
	private Connection udp;
	/**
	 * Window of the game
	 */
	private Window window;

	/**
	 * Buffer for received messages
	 */
	private ArrayList<Message> buffer;
	/**
	 * Buffer for sent messages
	 */
	private ArrayList<Message> sentMessages;
	/**
	 * Semaphore indicating that a buffer has a message in it
	 */
	private Semaphore m;

	/**
	 * Constant representing number of milliseconds to wait for acknowledgement before sending the message again
	 */
	private static int TIME_TO_ACK = 5000;

	/******************************************************************************************************************/

	/**
	 * Constructor for a thread that receives messages
	 *
	 * @param udp    connection
	 * @param window window of the game
	 */
	public Receiver(Connection udp, Window window) {
		this.udp = udp;
		this.window = window;
		buffer = new ArrayList<Message>();
		sentMessages = new ArrayList<Message>();
		m = new Semaphore(0);
		this.start();
	}

	/**
	 * Run method of the thread. Receives message from socket and adds it to the buffer
	 */
	@Override
	public void run() {
		while (true) {
			try {
				Message received = udp.receiveMessage();
				checkSentMessages();
				if (received.getType() != -1) {
					buffer.add(received);
					m.release();
				} else {
					System.err.println("Can't parse message");
				}
			} catch (IOException e) {
				checkSentMessages();
			}
		}
	}

	/**
	 * Checks buffer of sent messages for old ones and sends them again
	 */
	private void checkSentMessages() {
		for (int i = 0; i < sentMessages.size(); i++) {
			long now = new Date().getTime();
			if (now - sentMessages.get(i).getSentTime() > TIME_TO_ACK) {
				if (now - sentMessages.get(i).getSentTime() > 10 * TIME_TO_ACK) {
					System.err.println("Connection with server lost");
				} else if (now - sentMessages.get(i).getSentTime() > 3 * TIME_TO_ACK) {
					udp.sendMessage(new Message(udp.increaseNumberOfSentDatagrams(), 1, window.getNick().length(), window.getNick()), sentMessages);
				} Message m = sentMessages.remove(i);
				udp.sendMessage(m, sentMessages);
			}
		}
	}

	/**
	 * Getter for a message from a buffer
	 *
	 * @return a message from a buffer or null
	 */
	public Message getMessageFromBuffer() {
		try {
			m.acquire();
			return buffer.remove(0);
		} catch (InterruptedException e) {
			return null;
		}
	}

	/**
	 * Searches sent messages and removes the one that this acknowledgement is for
	 *
	 * @param ack acknowledgement
	 */
	public void ackMessage(Message ack) {
		for (int i = 0; i < sentMessages.size(); i++) {
			if (sentMessages.get(i).getNumber() == Integer.parseInt(ack.getData())) {
				sentMessages.remove(i);
				break;
			}
		}
	}

	/**
	 * Getter for sent messages
	 *
	 * @return list of sent messages
	 */
	public ArrayList<Message> getSentMessages() {
		return sentMessages;
	}
}
