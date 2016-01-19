import javax.swing.*;

/**
 * @author Jaroslav Klaus
 */
public class ProcessMessage extends Thread {
	Connection udp;
	private Receiver receiver;
	private Window window;

	/******************************************************************************************************************/

	public ProcessMessage(Connection udp, Receiver receiver, Window window) {
		this.udp = udp;
		this.receiver = receiver;
		this.window = window;
		this.start();
	}

	@Override
	public void run() {
		Message received;

		while (true) {
			received = receiver.getMessageFromBuffer();
			if (received.checkChecksum() && received.getNumber() == udp.getNumberOfReceivedDatagrams() + 1) {
				udp.increaseNumberOfReceivedDatagrams();
				sendAck(String.valueOf(received.getNumber()));

				switch (received.getType()) {
					case 2: // Ack
						receiver.ackMessage(received);
						receiver.zeroNoAck();
						break;
					case 6: // Answer to connect request
						respondType6(received);
						break;
					case 8: // Answer to reconnect request
						respondType8(received);
						break;
					case 9: // Start of game
						respondType9(received);
						break;
					case 11:    // Someone disconnected
						respondType11(received);
						break;
					case 12:    // You lost
						respondType12();
						break;
					case 13:    // Someone lost
						respondType13(received);
						break;
					case 14:    // You won (end of game, you guessed the word)
						respondType14();
						break;
					case 15:    // Someone won
						respondType15(received);
						break;
					case 16:    // Your move
						respondType16();
						break;
					case 17:    // Someone's move
						respondType17(received);
						break;
					case 19:    // Answer to move
						respondType19(received);
						break;
					case 20:    // Someone made a move
						respondType20(received);
						break;
					case 22:    // Someone tried to guess the word
						respondType22(received);
						break;
					default:
						break;
				}
			} else if (received.getNumber() <= udp.getNumberOfReceivedDatagrams()) {
				sendAck(String.valueOf(received.getNumber()));
			}
		}
	}

	private void sendAck(String receivedNumber) {
		udp.sendMessage(new Message(udp.increaseNumberOfSentDatagrams(), 2, receivedNumber.length(), receivedNumber),
				receiver.getSentMessages());
	}

	private void respondType6(Message received) {
		int data = Integer.parseInt(received.getData());
		if (data == 0) {
			window.setGame(new Game(window.getOpponents() + 1));
			window.setStatusLabelText("Connected");
		} else if (data == 1) {
			JOptionPane.showMessageDialog(window, "Player is already playing and is online", "Connection failed",
					JOptionPane.ERROR_MESSAGE);
		} else if (data == 2) {
			JOptionPane.showMessageDialog(window, "Player is already playing but is not available\nYou can reconnect "
					+ "to the game", "Connection failed", JOptionPane.INFORMATION_MESSAGE);
		}
	}

	private void respondType8(Message received) {
		String data[] = received.getData().split(",");
		if (Integer.parseInt(data[0]) == -1) {
			JOptionPane.showMessageDialog(window, "Game could not have been reconnected, nickname not found in any " +
					"game", "Reconnect not possible", JOptionPane.ERROR_MESSAGE);
		} else {
			window.setGame(new Game(Integer.parseInt(data[0]), Integer.parseInt(data[1]), Integer.parseInt(data[2]),
					data[3], Integer.parseInt(data[4])));
		}
	}

	private void respondType9(Message received) {
		String word = "";
		for (int i = 0; i < Integer.parseInt(received.getData()); i++) {
			word += "_";
		}
		window.setGuessedWordLabelText(word);
	}

	private void respondType11(Message received) {
		window.setStatusLabelText(received.getData() + " disconnected");
	}

	private void respondType12() {
		window.setStatusLabelText("End of game - you lost");
	}

	private void respondType13(Message received) {
		window.setStatusLabelText(received.getData() + " lost");
	}

	private void respondType14() {
		window.setStatusLabelText("End of game - you won");
	}

	private void respondType15(Message received) {
		window.setStatusLabelText("End of game - " + received.getData() + " won");
	}

	private void respondType16() {
		window.getGame().setMyMove(true);
		window.setStatusLabelText("It's your move");
		while (window.getGame().isMyMove()) {
			yield();
		}
		udp.sendMessage(window.getGame().getLastMessage(), receiver.getSentMessages());
	}

	private void respondType17(Message received) {
		window.setStatusLabelText("It's " + received.getData() + "'s move");
	}

	private void respondType19(Message received) {
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

	private void respondType20(Message received) {
		String data[] = received.getData().split(",");
		window.setStatusLabelText(data[0] + " guessed " + data[1]);
		String word = window.getGuessedWordLabelText(), newWord = "";
		for (int i = 0; i < data[2].length(); i++) {
			if (data[2].charAt(i) == '1') newWord += data[1];
			else newWord += word.charAt(i);
		}
		window.setGuessedWordLabelText(newWord);
	}

	private void respondType22(Message received) {
		String data[] = received.getData().split(",");
		window.setStatusLabelText(data[0] + " guessed word " + data[1]);
	}
}
