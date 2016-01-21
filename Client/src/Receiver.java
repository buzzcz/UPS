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
	 * Buffer for received messages
	 */
	private ArrayList<Message> buffer;
	private ArrayList<Message> sentMessages;
	/**
	 * Semaphore indicating that a buffer has a message in it
	 */
	private Semaphore m;

	private static int TIME_TO_ACK = 5000;

	/******************************************************************************************************************/

	/**
	 * Constructor for a thread that receives messages
	 */
	public Receiver(Connection udp) {
		this.udp = udp;
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
				System.err.println("Timeout");
				checkSentMessages();
			}
		}
	}

	private void checkSentMessages() {
		for (int i = 0; i < sentMessages.size(); i++) {
			long now = new Date().getTime();
			if (now - sentMessages.get(i).getSentTime() > TIME_TO_ACK) {
				if (now - sentMessages.get(i).getSentTime() > 10 * TIME_TO_ACK) {
					System.err.println("Connection lost with server");
				} else if (now - sentMessages.get(i).getSentTime() > 3 * TIME_TO_ACK) {
					udp.sendMessage(new Message(udp.increaseNumberOfSentDatagrams(), 1, 0, ""), sentMessages);
				}
				Message m = sentMessages.remove(i);
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
			e.printStackTrace();
			return null;
		}
	}

	public void ackMessage(Message ack) {
		for (int i = 0; i < sentMessages.size(); i++) {
			if (sentMessages.get(i).getNumber() == Integer.parseInt(ack.getData())) {
				sentMessages.remove(i);
				break;
			}
		}
	}

	public ArrayList<Message> getSentMessages() {
		return sentMessages;
	}
}
