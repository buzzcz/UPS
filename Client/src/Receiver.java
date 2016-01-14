import javax.swing.*;
import java.io.IOException;
import java.util.ArrayList;
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
	/**
	 * Buffer for received acknowledgements
	 */
	private ArrayList<Message> acks;
	/**
	 * Semaphore indicating that a buffer has a message in it
	 */
	private Semaphore m;
	/**
	 * Semaphore indicating that new ack has been received
	 */
	private Semaphore a;

	/******************************************************************************************************************/

	/**
	 * Constructor for a thread that receives messages
	 */
	public Receiver(Connection udp) {
		this.udp = udp;
		buffer = new ArrayList<Message>();
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
				if (received.getType() == 2) {
					acks.add(received);
					a.release();
				}
				else {
					buffer.add(received);
					m.release();
				}
			} catch (IOException e) {
				JOptionPane.showMessageDialog(null, "UDP receive error", "UDP error", JOptionPane.ERROR_MESSAGE);
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

	/**
	 * Getter for acknowledgement from a buffer
	 *
	 * @return acknowledgement from a buffer
	 */
	public Message getAckFromBuffer() {
		try {
			a.acquire();
			return acks.remove(0);
		} catch (InterruptedException e) {
			e.printStackTrace();
			return null;
		}
	}
}
