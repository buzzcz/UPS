import java.io.IOException;
import java.net.*;
import java.util.ArrayList;
import java.util.Date;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

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
	 * Address of the server
	 */
	private InetAddress host;
	/**
	 * Port of the server
	 */
	private int port;

	/**
	 * Number of sent datagrams by player
	 */
	private int sentDatagrams;
	/**
	 * Number of received datagrams by player
	 */
	private int receivedDatagrams;
	/**
	 * Lock for access to shared variable
	 */
	private Lock lock;

	/**
	 * Constant representing number of milliseconds to wait in receive
	 */
	static final int TIMEOUT = 5000;
	/**
	 * Constant for size of buffer receiving message
	 */
	private final int BUFFER_SIZE = 65000;

	private int numberOfReceived;
	private int numberOfUnparseable;
	private int numberOfSent;
	private int numberOfResent;

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
		lock = new ReentrantLock();
		sentDatagrams = 0;
		receivedDatagrams = 0;
		numberOfReceived = 0;
		numberOfUnparseable = 0;
		numberOfSent = 0;
		numberOfResent = 0;
	}

	/**
	 * Method that closes the socket
	 */
	public void close() {
		if (socket != null) socket.close();
	}

	/**
	 * Sends a datagram with specified message and adds it to the list of sent messages
	 *
	 * @param message      message to be sent
	 * @param sentMessages list of sent messages
	 */
	public void sendMessage(Message message, ArrayList<Message> sentMessages, boolean newTime) {
		DatagramPacket send = new DatagramPacket(message.getMessageByte(), message.getMessageByte().length, host,
				port);
		System.out.println("Client is sending: " + message.getMessage());
		try {
			lock.lock();
			socket.send(send);
			if (message.getType() != 1) {
				if (newTime) message.setSentTime(new Date().getTime());
				sentMessages.add(message);
			}
			numberOfSent++;
			lock.unlock();
		} catch (IOException e) {
			System.out.println("Message could not be sent");
		}
	}

	/**
	 * Receives a datagram with a message
	 *
	 * @return received message if OK, message with type -1 if parsing failed
	 * @throws IOException if timeout occurs
	 */
	public Message receiveMessage() throws IOException {
		byte[] buffer = new byte[BUFFER_SIZE];
		DatagramPacket receive = new DatagramPacket(buffer, buffer.length);
		socket.receive(receive);

		String s = new String(buffer);
		System.out.println("Server sent: " + s);
		numberOfReceived++;
		int index, number, type, checksum, dataSize;
		String data;

		try {
			index = s.indexOf(';');
			if (index == -1) throw new NumberFormatException();
			number = Integer.parseInt(s.substring(0, index));
			s = s.substring(index + 1);
			index = s.indexOf(';');
			if (index == -1) throw new NumberFormatException();
			type = Integer.parseInt(s.substring(0, index));
			s = s.substring(index + 1);
			index = s.indexOf(';');
			if (index == -1) throw new NumberFormatException();
			checksum = Integer.parseInt(s.substring(0, index));
			s = s.substring(index + 1);
			index = s.indexOf(';');
			if (index == -1) throw new NumberFormatException();
			dataSize = Integer.parseInt(s.substring(0, index));
			s = s.substring(index + 1);
			data = s.substring(0, dataSize);

			return new Message(number, type, checksum, dataSize, data);
		} catch (NumberFormatException e) {
			return new Message(-1, -1, -1, -1, "");
		}
	}

	/**
	 * Getter for number of sent datagrams
	 *
	 * @return number of sent datagrams
	 */
	public int getSentDatagrams() {
		return sentDatagrams;
	}

	/**
	 * Setter for number of sent datagrams
	 *
	 * @param sentDatagrams number of sent datagrams
	 */
	public void setSentDatagrams(int sentDatagrams) {
		this.sentDatagrams = sentDatagrams;
	}

	/**
	 * Returns number of sent datagrams and increments it
	 *
	 * @return number of sent datagrams
	 */
	public int increaseSentDatagrams() {
		return sentDatagrams++;
	}

	/**
	 * Getter for number of sent datagrams
	 *
	 * @return number of received datagrams
	 */
	public int getReceivedDatagrams() {
		return receivedDatagrams;
	}

	/**
	 * Setter for number of received datagrams
	 *
	 * @param receivedDatagrams number of received datagrams
	 */
	public void setReceivedDatagrams(int receivedDatagrams) {
		this.receivedDatagrams = receivedDatagrams;
	}

	/**
	 * Increments number of received datagrams
	 */
	public void increaseReceivedDatagrams() {
		receivedDatagrams++;
	}

	public int getNumberOfReceived() {
		return numberOfReceived;
	}

	public int getNumberOfUnparseable() {
		return numberOfUnparseable;
	}

	public int getNumberOfSent() {
		return numberOfSent;
	}

	public int getNumberOfResent() {
		return numberOfResent;
	}

	public void increaseNumberOfUnparseable() {
		numberOfUnparseable++;
	}

	public void increaseNumberOfResent() {
		numberOfResent++;
	}

	/**
	 * Getter for lock
	 *
	 * @return lock
	 */
	public Lock getLock() {
		return lock;
	}
}
