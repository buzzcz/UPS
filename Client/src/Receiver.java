import java.io.IOException;

/**
 * @author Jaroslav Klaus
 */
public class Receiver extends Thread {
	/**
	 * Connection
	 * */
	private Connection udp;

	/**
	 * Constructor for a thread that receives messages
	 * */
	public Receiver(Connection udp) {
		this.udp = udp;
		this.start();
	}

	@Override
	public void run() {
		while (true) {
			try {
				udp.receiveMessage();
			} catch (IOException e) {
				System.out.println("UDP receiver error");
			}
		}
	}
}
