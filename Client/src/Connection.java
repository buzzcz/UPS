import java.io.IOException;
import java.net.*;

/**
 * Class for UDP connection and communication
 *
 * @author Jaroslav Klaus
 */
public class Connection {
	/**
	 * UDP socket
	 */
	private DatagramSocket socket;
	/**
	 * Host address of the socket
	 */
	private InetAddress host;
	/**
	 * Port of the socket
	 */
	private int port;

	/**
	 * Number of sent datagrams
	 */
	private int numberOfSentDatagrams;
	/**
	 * Number of received datagrams
	 */
	private int numberOfReceivedDatagrams;

	/******************************************************************************************************************/

	/**
	 * Constructor for creating a connection
	 *
	 * @param host host address of the socket
	 * @param port port of the socket
	 */
	public Connection(String host, int port) {
		try {
			this.host = InetAddress.getByName(host);
		} catch (UnknownHostException e) {
			System.out.println("Host " + host + " unknown");
		}
		this.port = port;
		try {
			socket = new DatagramSocket();
		} catch (SocketException e) {
			System.out.println("Socket could not be made");
		}
		try {
			socket.setSoTimeout(3000);
		} catch (SocketException e) {
			System.out.println("Socket timeout could not be set");
		}
	}

	/**
	 * Method that closes the socket
	 */
	public void close() {
		if (socket != null) socket.close();
	}

	/**
	 * Sends a datagram with specified message
	 *
	 * @param message message to be sent
	 */
	public void sendMessage(Message message) {
		DatagramPacket send = new DatagramPacket(message.getMessageByte(), message.getMessageByte().length, host,
				port);
		try {
			socket.send(send);
			numberOfSentDatagrams++;
		} catch (IOException e) {
			System.out.println("Message could not be sent");
		}
	}

	private int[] peekMessage() {
//		TODO peek message and return bytes to read and message size
		return null;
	}

	/**
	 * Receives a datagram with a message
	 *
	 * @return received message
	 */
	public Message receiveMessage() throws IOException {
//		TODO buffer size??
		byte[] buffer = new byte[256];
		DatagramPacket receive = new DatagramPacket(buffer, buffer.length);
		socket.receive(receive);
		numberOfReceivedDatagrams++;

		String s = new String(buffer);
		int number, type, checksum, dataSize;
		String data;

		number = Integer.parseInt(s.substring(0, s.indexOf(';')));
		s = s.substring(s.indexOf(';') + 1);
		type = Integer.parseInt(s.substring(0, s.indexOf(';')));
		s = s.substring(s.indexOf(';') + 1);
		checksum = Integer.parseInt(s.substring(0, s.indexOf(';')));
		s = s.substring(s.indexOf(';') + 1);
		dataSize = Integer.parseInt(s.substring(0, s.indexOf(';')));
		data = s;

		return new Message(number, type, checksum, dataSize, data);
	}

	/**
	 * Getter for number of sent datagrams
	 *
	 * @return number of sent datagrams
	 */
	public int getNumberOfSentDatagrams() {
		return numberOfSentDatagrams;
	}

	/**
	 * Getter for number of sent datagrams
	 *
	 * @return number of received datagrams
	 */
	public int getNumberOfReceivedDatagrams() {
		return numberOfReceivedDatagrams;
	}
}
