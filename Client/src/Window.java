import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Arrays;

/**
 * @author Jaroslav Klaus
 */
public class Window extends JFrame {
	/**
	 * Connection
	 */
	private Connection udp;
	/**
	 * Thread that will receive messages
	 */
	private Receiver receiver;
	/**
	 * Thread that will process messages
	 */
	private ProcessMessage consumer;
	/**
	 * Thread that will send ping messages
	 */
	private Thread ping;
	/**
	 * Game in which a player is
	 */
	private Game game;
	/**
	 * Nickname selected in new game dialog
	 */
	private String nick;
	/**
	 * Number of opponents selected in new game dialog
	 */
	private int opponents;
	/**
	 * Last guessed char or word
	 */
	private String lastGuessed;

	/**
	 * Headline label
	 */
	private JLabel statusLabel;
	/**
	 * Canvas for painting the hangman
	 */
	private Canvas canvas;
	/**
	 * Label with the guessed word
	 */
	private JLabel guessedWordLabel;
	/**
	 * Label with already guessed letters
	 */
	private JLabel alreadyGuessedLabel;

	/******************************************************************************************************************/

	/**
	 * Constructor for a window
	 */
	public Window() {
		initWindow();
		this.setVisible(true);

		String host = null;
		int port = 0;
		boolean ok = false;
		while (!ok) {
			JTextField addressField = new JTextField();
			SpinnerNumberModel sModel = new SpinnerNumberModel(1, 1, 65535, 1);
			JSpinner portSpinner = new JSpinner(sModel);
			JComponent[] components = new JComponent[]{new JLabel("Enter server address:"), addressField, new JLabel
					("Enter server port:"), portSpinner};

			int option = JOptionPane.showOptionDialog(Window.this, components, "Connection", JOptionPane
					.OK_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE, null, null, null);
			if (option == JOptionPane.OK_OPTION) {
				host = addressField.getText();
				if (!host.trim().isEmpty()) ok = true;
				port = Integer.parseInt(portSpinner.getValue().toString());
			} else {
				exitClient();
			}
		}

		udp = new Connection(host, port);
		receiver = new Receiver(udp, this);
		consumer = new ProcessMessage(udp, receiver, this);
		ping = new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						if (game != null) {
							udp.getLock().lock();
							boolean sent = false;
							for (int i = 0; i < receiver.getSentMessages().size(); i++) {
								if (receiver.getSentMessages().get(i).getType() == 21) {
									udp.sendMessage(receiver.getSentMessages().get(i), receiver.getSentMessages(),
											false);
									sent = true;
								}
							}
							if (!sent) {
								Message m = new Message(udp.getSentDatagrams(), 21, nick.length(), nick);
								udp.sendMessage(m, receiver.getSentMessages(), true);
							}
							udp.getLock().unlock();
						}
						Thread.sleep(Receiver.TIME_TO_ACK + 1000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		});
		ping.start();
		game = null;
		nick = null;
		opponents = 0;
	}

	/**
	 * Initializes window and creates its layout and adds listeners
	 */
	private void initWindow() {
		setTitle("Hangman");
		setSize(800, 600);
		setLocationRelativeTo(null);
		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				exitClient();
			}
		});

		createMenu();
		createLayout();
		registerListeners();
	}

	private void printStats() {
		if (udp != null) {
			System.out.println("Received: " + udp.getNumberOfReceived() + "\nUnparseable: " + udp
					.getNumberOfUnparseable() + "\nSent: " + udp.getNumberOfSent() + "\nResent: " + udp
					.getNumberOfResent() + "\nReceived bytes: " +

					udp.getBytesReceived() + "\nSent bytes: " + udp.getBytesSent() + "\n");
		}
	}

	/**
	 * Method that closes the socket and exits the game
	 */
	public void exitClient() {
		int result = JOptionPane.showOptionDialog(Window.this, "Do you want to exit the game?", "Exit game",
				JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE, null, null, null);
		if (result == JOptionPane.YES_OPTION) {
			if (game != null) {
				Message m = new Message(udp.increaseSentDatagrams(), 8, nick.length(), nick);
				udp.sendMessage(m, receiver.getSentMessages(), true);
			}

			printStats();
			System.out.println("Interrupting threads...");
			if (receiver != null) receiver.interrupt();
			if (consumer != null) consumer.interrupt();
			System.out.println("Exiting...");
			if (udp != null) udp.close();
			System.exit(0);
		}
	}

	/**
	 * Creates menu bar, menu items and actions
	 */
	private void createMenu() {
		JMenuBar menu = new JMenuBar();

		final JMenu gameMenu = new JMenu("Game");
		gameMenu.setMnemonic(KeyEvent.VK_G);
		final JMenuItem newGame = new JMenuItem("New Game");
		newGame.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, KeyEvent.CTRL_MASK));
		newGame.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				boolean ok = false;
				while (!ok) {
					JTextField nickField = new JTextField();
					SpinnerNumberModel sModel = new SpinnerNumberModel(1, 1, Integer.MAX_VALUE, 1);
					JSpinner opponentsSpinner = new JSpinner(sModel);
					JComponent[] components = new JComponent[]{new JLabel("Enter nickname:"), nickField, new JLabel
							("Enter number of opponents:"), opponentsSpinner};
					int option = JOptionPane.showOptionDialog(Window.this, components, "New game", JOptionPane
							.OK_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE, null, null, null);
					if (option == JOptionPane.OK_OPTION) {
						nick = nickField.getText();
						if (!nick.trim().isEmpty()) {
							ok = true;
							udp.setSentDatagrams(0);
							udp.setReceivedDatagrams(0);
							opponents = (Integer) opponentsSpinner.getValue();
							Message m = new Message(udp.increaseSentDatagrams(), 3, nick.length() +
									opponentsSpinner.getValue().toString().length() + 1, nick + "," +
									opponentsSpinner.getValue().toString());
							udp.sendMessage(m, receiver.getSentMessages(), true);
						}
					} else ok = true;
				}
			}
		});
		JMenuItem reconnect = new JMenuItem("Reconnect");
		reconnect.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, KeyEvent.CTRL_MASK));
		reconnect.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				JTextField nickField = new JTextField();
				JComponent[] components = new JComponent[]{new JLabel("Enter nickname:"), nickField};
				int result = JOptionPane.showOptionDialog(Window.this, components, "Reconnect", JOptionPane
						.OK_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE, null, null, null);
				if (result == JOptionPane.OK_OPTION) {
					nick = nickField.getText();
					udp.setSentDatagrams(0);
					udp.setReceivedDatagrams(0);
					Message m = new Message(udp.increaseSentDatagrams(), 5, nick.length(), nick);
					udp.sendMessage(m, receiver.getSentMessages(), true);
				}
			}
		});
		JMenuItem exit = new JMenuItem("Exit");
		exit.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q, KeyEvent.CTRL_MASK));
		exit.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				exitClient();
			}
		});
		gameMenu.add(newGame);
		gameMenu.add(reconnect);
		gameMenu.add(exit);

		menu.add(gameMenu);

		setJMenuBar(menu);
	}

	/**
	 * Creates layout of the window
	 */
	private void createLayout() {
		getContentPane().setLayout(new BoxLayout(getContentPane(), BoxLayout.PAGE_AXIS));
		statusLabel = new JLabel("No Game");
		statusLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
		getContentPane().add(statusLabel);
		getContentPane().add(new JSeparator(SwingConstants.HORIZONTAL));

		canvas = new Canvas(11);
		getContentPane().add(canvas);
		getContentPane().add(new JSeparator(SwingConstants.HORIZONTAL));

		guessedWordLabel = new JLabel("No word");
		guessedWordLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
		getContentPane().add(guessedWordLabel);
		getContentPane().add(new JSeparator(SwingConstants.HORIZONTAL));

		alreadyGuessedLabel = new JLabel("No letters");
		alreadyGuessedLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
		getContentPane().add(alreadyGuessedLabel);

		canvas.setPreferredSize(new Dimension(getWidth(), getHeight() - statusLabel.getHeight() - guessedWordLabel
				.getHeight() - alreadyGuessedLabel.getHeight()));
		addComponentListener(new ComponentAdapter() {
			@Override
			public void componentResized(ComponentEvent e) {
				resizeLabel(statusLabel);
				resizeLabel(guessedWordLabel);
				resizeLabel(alreadyGuessedLabel);
				canvas.setPreferredSize(new Dimension(getWidth(), getHeight() - statusLabel.getHeight() -
						guessedWordLabel.getHeight() - alreadyGuessedLabel.getHeight()));
			}
		});
		statusLabel.addPropertyChangeListener("text", new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				resizeLabel(statusLabel);
			}
		});
		guessedWordLabel.addPropertyChangeListener("text", new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				resizeLabel(guessedWordLabel);
			}
		});
		alreadyGuessedLabel.addPropertyChangeListener("text", new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				resizeLabel(alreadyGuessedLabel);
			}
		});
		statusLabel.setFont(new Font(statusLabel.getFont().getName(), Font.PLAIN, 60));
		guessedWordLabel.setFont(new Font(guessedWordLabel.getFont().getName(), Font.PLAIN, 60));
		alreadyGuessedLabel.setFont(new Font(alreadyGuessedLabel.getFont().getName(), Font.PLAIN, 60));
	}

	private void resizeLabel(JLabel label) {
		if (label.getText().length() * label.getFont().getSize() > getWidth()) {
			while (label.getText().length() * label.getFont().getSize() > getWidth()) {
				label.setFont(new Font(label.getFont().getName(), Font.PLAIN, label.getFont().getSize() - 1));
			}
		} else if (label.getText().length() * (label.getFont().getSize() + 1) < getWidth() * 0.9 && label.getFont()
				.getSize() + 1 < 60) {
			while (label.getText().length() * (label.getFont().getSize() + 1) < getWidth() * 0.9 && label.getFont()
					.getSize() + 1 < 60) {
				label.setFont(new Font(label.getFont().getName(), Font.PLAIN, label.getFont().getSize() + 1));
			}
		}
		canvas.setPreferredSize(new Dimension(getWidth(), getHeight() - statusLabel.getHeight() - guessedWordLabel
				.getHeight() - alreadyGuessedLabel.getHeight()));
	}

	/**
	 * Registers keyboard listener
	 */
	private void registerListeners() {
		addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				if (alreadyGuessedLabel.getText().contains((e.getKeyChar() + "").toUpperCase())) {
					JOptionPane.showMessageDialog(Window.this, (e.getKeyChar() + "").toUpperCase() + " has already " +
							"been guessed", "Letter already guessed", JOptionPane.ERROR_MESSAGE);
				} else if (game != null && game.isMyMove() && ((e.getKeyChar() >= 'a' && e.getKeyChar() <= 'z') || e
						.getKeyChar() == '\'' || e.getKeyChar() == ' ')) {
					String data = e.getKeyChar() + "";
					Message m = new Message(udp.increaseSentDatagrams(), 16, nick.length() + 1 + data.length(), nick +
							"," + data.toUpperCase());
					udp.sendMessage(m, receiver.getSentMessages(), true);
					lastGuessed = data.toUpperCase();
					setStatusLabelText("You guessed " + lastGuessed);
					if (!alreadyGuessedLabel.getText().contains(lastGuessed)) {
						char[] ch = (alreadyGuessedLabel.getText() + lastGuessed).toCharArray();
						Arrays.sort(ch);
						alreadyGuessedLabel.setText(new String(ch));
					}
					game.setMyMove(false);
				} else if (game != null && game.isMyMove() && e.getKeyCode() == KeyEvent.VK_ENTER) {
					JTextField wordField = new JTextField();
					JComponent[] components = new JComponent[]{new JLabel("Enter guessed word:"), wordField};
					int option = JOptionPane.showOptionDialog(Window.this, components, "Guessing word", JOptionPane
							.OK_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE, null, null, null);
					if (option == JOptionPane.OK_OPTION) {
						Message m = new Message(udp.increaseSentDatagrams(), 19, nick.length() + 1 + wordField.getText
								().length(), nick + "," + wordField.getText().toUpperCase());
						udp.sendMessage(m, receiver.getSentMessages(), true);
						lastGuessed = wordField.getText().toUpperCase();
						setStatusLabelText("You guessed " + wordField.getText().toUpperCase());
						game.setMyMove(false);
					}
				}
			}
		});
	}

	/**
	 * Getter for a game in which a player is
	 *
	 * @return game in which a player is
	 */
	public Game getGame() {
		return game;
	}

	/**
	 * Setter for a game in which a player is
	 *
	 * @param game game in which a player is
	 */
	public void setGame(Game game) {
		this.game = game;
	}

	/**
	 * Getter for a number of opponents selected in new game dialog
	 *
	 * @return number of opponents selected in new game dialog
	 */
	public int getOpponents() {
		return opponents;
	}

	/**
	 * Sets text into status label
	 *
	 * @param text text to be sent into status label
	 */
	public void setStatusLabelText(String text) {
		statusLabel.setText(text);
	}

	/**
	 * Getter for text in word label
	 *
	 * @return text in word label
	 */
	public String getGuessedWordLabelText() {
		return guessedWordLabel.getText();
	}

	/**
	 * Getter for canvas
	 *
	 * @return canvas
	 */
	public Canvas getCanvas() {
		return canvas;
	}

	/**
	 * Sets text into guessed word label
	 *
	 * @param text text to be sent into guessed word label
	 */
	public void setGuessedWordLabelText(String text) {
		guessedWordLabel.setText(text);
	}

	/**
	 * Getter for label with already guessed letters
	 *
	 * @return label with already guessed letters
	 */
	public JLabel getAlreadyGuessedLabel() {
		return alreadyGuessedLabel;
	}

	/**
	 * Getter for last guessed letter / word
	 *
	 * @return last guessed letter / word
	 */
	public String getLastGuessed() {
		return lastGuessed;
	}

	/**
	 * Sets last guessed letter / word
	 *
	 * @param lastGuessed last guessed letter / word
	 */
	public void setLastGuessed(String lastGuessed) {
		this.lastGuessed = lastGuessed;
	}

	/**
	 * Getter for player's nick
	 *
	 * @return player's nick
	 */
	public String getNick() {
		return nick;
	}
}
