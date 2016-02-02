/**
 * Class for message
 *
 * @author Jaroslav Klaus
 */
public class Message {
	/**
	 * Sequence number of datagram
	 */
	private int number;
	/**
	 * Type of data in datagram
	 */
	private int type;
	/**
	 * Checksum
	 */
	private int checksum;
	/**
	 * Size of data in datagram
	 */
	private int dataSize;
	/**
	 * Data in datagram
	 */
	private String data;
	/**
	 * Timestamp
	 */
	private long sentTime;

	/******************************************************************************************************************/

	/**
	 * Constructor for creating new message
	 *
	 * @param number   sequence number of datagram
	 * @param type     type of data in datagram
	 * @param dataSize Size of data in datagram
	 * @param data     Data in datagram
	 */
	public Message(int number, int type, int dataSize, String data) {
		this.number = number;
		this.type = type;
		this.dataSize = dataSize;
		this.data = data;
		this.checksum = number + type + dataSize;
		for (int i = 0; i < dataSize; i++) checksum += data.charAt(i);
		checksum %= 256;
	}

	/**
	 * Constructor for creating new message
	 *
	 * @param number   sequence number of datagram
	 * @param type     type of data in datagram
	 * @param checksum checksum
	 * @param dataSize Size of data in datagram
	 * @param data     Data in datagram
	 */
	public Message(int number, int type, int checksum, int dataSize, String data) {
		this.number = number;
		this.type = type;
		this.checksum = checksum;
		this.dataSize = dataSize;
		this.data = data;
	}

	/**
	 * Getter for message
	 *
	 * @return message
	 */
	public String getMessage() {
		return number + ";" + type + ";" + checksum + ";" + dataSize + ";" + data;
	}

	/**
	 * Getter for message bytes
	 *
	 * @return message bytes
	 */
	public byte[] getMessageByte() {
		return getMessage().getBytes();
	}

	/**
	 * Checks whether checksum is correct
	 *
	 * @return <code>true</code> if checksum is correct; <code>false</code> otherwise
	 */
	public boolean checkChecksum() {
		int checksum = number + type + dataSize;
		for (int i = 0; i < dataSize; i++) checksum += data.charAt(i);
		checksum %= 256;

		return checksum == this.checksum;
	}

	/**
	 * Checks if size of received data is alright based on type of message
	 *
	 * @return <code>true</code> if ok;<code>false</code> otherwise
	 */
	public boolean checkData() {
		switch (type) {
			case 1: // Ack
			case 4: // Answer to connect request
				if (dataSize <= 0) return false;
				try {
					Integer.parseInt(data);
				} catch (NumberFormatException e) {
					return false;
				}
				return true;
			case 2: // Unreachable client
			case 6: // Answer to reconnect request
			case 7: // Start of game
			case 9: // Someone disconnected
			case 11:    // Someone lost
			case 13:    // Someone won
			case 15:    // Someone's move
			case 17:    // Answer to move
			case 18:    // Someone made a move
			case 20:    // Someone tried to guess the word
			case 22:    // Not responding player
				if (dataSize <= 0) return false;
				return true;
			case 10:    // You lost
			case 12:    // You won (end of game, you guessed the word)
			case 14:    // Your move
			case 21:    // Ping
				if (dataSize != 0) return false;
				return true;
			default:
				System.out.println("Unknown message type " + type);
				return false;
		}
	}

	/**
	 * Getter for a number of message
	 *
	 * @return number of message
	 */
	public int getNumber() {
		return number;
	}

	/**
	 * Getter for a type of message
	 *
	 * @return type of message
	 */
	public int getType() {
		return type;
	}

	/**
	 * Getter for a message checksum
	 *
	 * @return message checksum
	 */
	public int getChecksum() {
		return checksum;
	}

	/**
	 * Getter for a size of data in message
	 *
	 * @return size of data in message
	 */
	public int getDataSize() {
		return dataSize;
	}

	/**
	 * Getter far data in message
	 *
	 * @return data in message
	 */
	public String getData() {
		return data;
	}

	/**
	 * Getter fot sent time
	 *
	 * @return sent time
	 */
	public long getSentTime() {
		return sentTime;
	}

	/**
	 * Setter for sent time
	 *
	 * @param sentTime sent time
	 */
	public void setSentTime(long sentTime) {
		this.sentTime = sentTime;
	}
}
