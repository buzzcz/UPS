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
		return number + "+" + type + "+" + dataSize + ";" + checksum + ";" + data;
	}

	/**
	 * Getter for message bytes
	 *
	 * @return message bytes
	 */
	public byte[] getMessageByte() {
		return (number + "+" + type + "+" + dataSize + ";" + checksum + ";" + data).getBytes();
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
}
