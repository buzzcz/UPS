/**
 * Class for message
 *
 * @author Jaroslav Klaus
 */
public class Message {
	/**
	 * Message to be sent or received
	 */
	private String message;

	/******************************************************************************************************************/

	/**
	 * Constructor for creating new message
	 *
	 * @param message new message
	 */
	public Message(String message) {
		this.message = message;
	}

	/**
	 * Getter for message
	 *
	 * @return message
	 */
	public String getMessage() {
		return message;
	}

	/**
	 * Getter for message bytes
	 *
	 * @return message bytes
	 */
	public byte[] getMessageByte() {
		if (message != null) return message.getBytes();
		else return null;
	}
}
