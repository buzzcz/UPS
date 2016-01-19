import java.io.IOException;
import java.net.*;
import java.util.ArrayList;
import java.util.Date;

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

	static final int TIMEOUT = 5000;
	private final int BUFFER_SIZE = 1024;

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
			socket.setSoTimeout(TIMEOUT);
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
	public void sendMessage(Message message, ArrayList<Message> sentMessages) {
		DatagramPacket send = new DatagramPacket(message.getMessageByte(), message.getMessageByte().length, host,
				port);
		System.out.println("Client is sending: " + message.getMessage());
		try {
			socket.send(send);
			if (message.getType() != 2) {
				message.setSentTime(new Date().getTime());
				sentMessages.add(message);
			}
		} catch (IOException e) {
			System.out.println("Message could not be sent");
		}
	}

	/**
	 * Receives a datagram with a message
	 *
	 * @return received message
	 */
	public Message receiveMessage() throws IOException {
		byte[] buffer = new byte[BUFFER_SIZE];
		DatagramPacket receive = new DatagramPacket(buffer, buffer.length);
		socket.receive(receive);

		String s = new String(buffer);
		System.out.println("Server sent: " + s);
		int index, number, type, checksum, dataSize;
		String data;

		try {
			index = s.indexOf(';');
			if (index == -1) throw new NumberFormatException("Number not found");
			number = Integer.parseInt(s.substring(0, index));
			s = s.substring(index + 1);
			index = s.indexOf(';');
			if (index == -1) throw new NumberFormatException("Type not found");
			type = Integer.parseInt(s.substring(0, index));
			s = s.substring(index + 1);
			index = s.indexOf(';');
			if (index == -1) throw new NumberFormatException("Checksum not found");
			checksum = Integer.parseInt(s.substring(0, index));
			s = s.substring(index + 1);
			index = s.indexOf(';');
			if (index == -1) throw new NumberFormatException("Data size not found");
			dataSize = Integer.parseInt(s.substring(0, index));
			s = s.substring(index + 1);
			data = s;

			return new Message(number, type, checksum, dataSize, data);
		} catch (NumberFormatException e) {
			return new Message(-1, -1, -1, "");
		}
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
	 * Returns number of sent datagrams and increments it
	 *
	 * @return number of sent datagrams
	 */
	public int increaseNumberOfSentDatagrams() {
		return numberOfSentDatagrams++;
	}

	/**
	 * Getter for number of sent datagrams
	 *
	 * @return number of received datagrams
	 */
	public int getNumberOfReceivedDatagrams() {
		return numberOfReceivedDatagrams;
	}

	/**
	 * Increments number of received datagrams
	 */
	public void increaseNumberOfReceivedDatagrams() {
		numberOfReceivedDatagrams++;
	}
}
