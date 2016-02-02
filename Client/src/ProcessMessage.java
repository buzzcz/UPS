import javax.swing.*;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * @author Jaroslav Klaus
 */
public class ProcessMessage extends Thread {
	/**
	 * Connection
	 */
	Connection udp;
	/**
	 * Thread that receives messages
	 */
	private Receiver receiver;
	/**
	 * Window of the game
	 */
	private Window window;

	/******************************************************************************************************************/

	/**
	 * Constructor for a thread that processes messages
	 *
	 * @param udp      connection
	 * @param receiver thread that receives messages
	 * @param window   window of the game
	 */
	public ProcessMessage(Connection udp, Receiver receiver, Window window) {
		this.udp = udp;
		this.receiver = receiver;
		this.window = window;
		this.start();
	}

	/**
	 * Run method of the thread. Gets message from a buffer and processes it
	 */
	@Override
	public void run() {
		Message received;

		try {
			while (true) {
				received = receiver.getMessageFromBuffer();
				udp.getLock().lock();
				if (window.getGame() != null || received.getType() == 1 || received.getType() == 4 || received.getType
						() == 6) {
					if (received.checkChecksum()) {
						if (received.getNumber() == udp.getReceivedDatagrams() + 1 || received.getType() == 1 ||
								received.getType() == 4 || received.getType() == 6 || received.getType() == 21) {

							if (received.getType() != 1) {
								if (received.getType() != 21) udp.increaseReceivedDatagrams();
								sendAck(String.valueOf(received.getNumber()));
							}

							switch (received.getType()) {
								case 1: // Ack
									receiver.ackMessage(received);
									break;
								case 2: // Unreachable client
									respondType2(received);
									break;
								case 4: // Answer to connect request
									respondType4(received);
									break;
								case 6: // Answer to reconnect request
									respondType6(received);
									break;
								case 7: // Start of game
									respondType7(received);
									break;
								case 9: // Someone disconnected
									respondType9(received);
									break;
								case 10:    // You lost
									respondType10();
									break;
								case 11:    // Someone lost
									respondType11(received);
									break;
								case 12:    // You won (end of game, you guessed the word)
									respondType12();
									break;
								case 13:    // Someone won
									respondType13(received);
									break;
								case 14:    // Your move
									respondType14();
									break;
								case 15:    // Someone's move
									respondType15(received);
									break;
								case 17:    // Answer to move
									respondType17(received);
									break;
								case 18:    // Someone made a move
									respondType18(received);
									break;
								case 20:    // Someone tried to guess the word
									respondType20(received);
									break;
								case 21:    // Ping
									break;
								case 22:    // Not responding player
									respondType22(received);
									break;
								default:
									System.out.println("Unknown type");
									break;
							}
						} else if (received.getNumber() <= udp.getReceivedDatagrams()) {
							sendAck(String.valueOf(received.getNumber()));
						} else
							System.out.println("Number not correct: " + received.getNumber() + ", expecting: " + (udp
									.getReceivedDatagrams() + 1));
					} else System.out.println("Wrong checksum");
				}
				udp.getLock().unlock();
			}
		} catch (Exception e) {

		}
	}

	/**
	 * Sends acknowledgement to a given packet number
	 *
	 * @param receivedNumber number that should be acknowledged
	 */
	private void sendAck(String receivedNumber) {
		udp.sendMessage(new Message(udp.getSentDatagrams(), 1, window.getNick().length() + 1 +
				receivedNumber.length(), window.getNick() + "," + receivedNumber), receiver.getSentMessages(), false);
	}

	private void respondType2(Message received) {
		window.setStatusLabelText("End of game - " + received.getData() + " is unreachable");
		window.setGame(null);
		receiver.setSentMessages(new ArrayList<Message>());
	}

	private void respondType4(Message received) {
		int data = Integer.parseInt(received.getData());
		if (data == 0) {
			window.setGame(new Game(window.getOpponents() + 1));
			window.setStatusLabelText("Connected, waiting for other players");
		} else if (data == 1) {
			JOptionPane.showMessageDialog(window, "Player is already playing and is online", "Connection failed",
					JOptionPane.ERROR_MESSAGE);
		} else if (data == 2) {
			JOptionPane.showMessageDialog(window, "Player is already playing but is not available\nYou can reconnect "
					+ "to the game", "Connection failed", JOptionPane.INFORMATION_MESSAGE);
		}
	}

	private void respondType6(Message received) {
		String data[] = received.getData().split(",");
		if (Integer.parseInt(data[0]) == -1) {
			JOptionPane.showMessageDialog(window, "Game could not have been reconnected, player not found in any " +
					"game", "Reconnect not possible", JOptionPane.ERROR_MESSAGE);
			return;
		} else if (Integer.parseInt(data[0]) == -2) {
			JOptionPane.showMessageDialog(window, "Game could not have been reconnected, player is online",
					"Reconnect" + " not possible", JOptionPane.ERROR_MESSAGE);
			return;
		} else if (Integer.parseInt(data[0]) == 0) {
			window.setGame(new Game(Integer.parseInt(data[1])));
		} else {
			String letters = "";
			for (int i = 0; i < data[3].length(); i++) {
				if (data[3].charAt(i) != '0') letters += data[3].charAt(i);
			}
			window.setGame(new Game(Integer.parseInt(data[0]), Integer.parseInt(data[1]), data[2].length(), letters,
					Integer.parseInt(data[4])));
			window.setGuessedWordLabelText(data[2]);
			window.getAlreadyGuessedLabel().setText(letters);
			window.getCanvas().setWrongGuesses(Integer.parseInt(data[4]));
		}
		window.setStatusLabelText("Reconnected, waiting for other players");
	}

	private void respondType7(Message received) {
		String word = "";
		for (int i = 0; i < Integer.parseInt(received.getData()); i++) {
			word += "-";
		}
		window.setStatusLabelText("Start of the game");
		window.setGuessedWordLabelText(word);
		window.getAlreadyGuessedLabel().setText(" ");
		window.getCanvas().setWrongGuesses(0);
	}

	private void respondType9(Message received) {
		window.setStatusLabelText(received.getData() + " disconnected");
		window.getGame().setMyMove(false);
	}

	private void respondType10() {
		window.setStatusLabelText("End of game - you lost");
	}

	private void respondType11(Message received) {
		window.setStatusLabelText(received.getData() + " lost");
	}

	private void respondType12() {
		window.setStatusLabelText("End of game - you won");
		if (window.getLastGuessed().length() > 1) {
			window.setGuessedWordLabelText(window.getLastGuessed());
		}
		window.setGame(null);
		receiver.setSentMessages(new ArrayList<Message>());
	}

	private void respondType13(Message received) {
		window.setStatusLabelText("End of game - " + received.getData() + " won");
		if (window.getGuessedWordLabelText().contains("_")) window.setGuessedWordLabelText(window.getLastGuessed());
		window.setGame(null);
		receiver.setSentMessages(new ArrayList<Message>());
	}

	private void respondType14() {
		window.getGame().setMyMove(true);
		window.setStatusLabelText("It's your move");
	}

	private void respondType15(Message received) {
		window.setStatusLabelText("It's " + received.getData() + "'s move");
	}

	private void respondType17(Message received) {
		boolean hit = false;
		String word = window.getGuessedWordLabelText(), newWord = "";
		for (int i = 0; i < received.getData().length(); i++) {
			if (received.getData().charAt(i) == '1') {
				newWord += window.getLastGuessed();
				hit = true;
			} else newWord += word.charAt(i);
		}
		if (!hit) {
			window.getGame().wrongGuess();
			window.getCanvas().wrongGuess();
		}
		window.setGuessedWordLabelText(newWord);
	}

	private void respondType18(Message received) {
		String data[] = received.getData().split(",");
		window.setStatusLabelText(data[0] + " guessed " + data[1]);
		String word = window.getGuessedWordLabelText(), newWord = "";
		for (int i = 0; i < data[2].length(); i++) {
			if (data[2].charAt(i) == '1') newWord += data[1];
			else newWord += word.charAt(i);
		}
		window.setGuessedWordLabelText(newWord);
		if (!window.getAlreadyGuessedLabel().getText().contains(data[1])) {
			char[] ch = (window.getAlreadyGuessedLabel().getText() + data[1]).toCharArray();
			Arrays.sort(ch);
			window.getAlreadyGuessedLabel().setText(new String(ch));
		}
	}

	private void respondType20(Message received) {
		String data[] = received.getData().split(",");
		window.setStatusLabelText(data[0] + " guessed word " + data[1]);
		window.setLastGuessed(data[1]);
	}

	private void respondType22(Message received) {
		window.setStatusLabelText("Player " + received.getData() + " is not responding for a while");
	}
}
