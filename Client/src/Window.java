import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

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
	 * Last guessed char
	 */
	private char lastGuessed;

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

	/******************************************************************************************************************/

	/**
	 * Constructor for a window
	 *
	 * @param host host address
	 * @param port port of socket
	 */
	public Window(String host, int port) {
		initWindow();
		udp = new Connection(host, port);
		receiver = new Receiver(udp, this);
		consumer = new ProcessMessage(udp, receiver, this);
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

	/**
	 * Method that closes the socket and exits the game
	 */
	public void exitClient() {
		int result = JOptionPane.showOptionDialog(Window.this, "Do you want to exit the game?", "Exit game",
				JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE, null, null, null);
		if (result == JOptionPane.YES_OPTION) {
//		    TODO how to exit game - when and how to stop threads
			receiver.interrupt();
			consumer.interrupt();
			if (game != null) {
				Message m = new Message(udp.increaseNumberOfSentDatagrams(), 10, nick.length(), nick);
				udp.sendMessage(m, receiver.getSentMessages());
			}
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
		JMenuItem newGame = new JMenuItem("New Game");
		newGame.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, KeyEvent.CTRL_MASK));
		newGame.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				JTextField nickField = new JTextField();
				SpinnerNumberModel sModel = new SpinnerNumberModel(1, 1, Integer.MAX_VALUE, 1);
				JSpinner opponentsSpinner = new JSpinner(sModel);
				JComponent[] components = new JComponent[]{new JLabel("Enter nickname:"), nickField, new JLabel
						("Enter" +
						" " + "number of opponents:"), opponentsSpinner};
				int option = JOptionPane.showOptionDialog(Window.this, components, "New game", JOptionPane
						.OK_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE, null, null, null);
				if (option == JOptionPane.OK_OPTION) {
					nick = nickField.getText();
					opponents = (Integer) opponentsSpinner.getValue();
					Message m = new Message(udp.increaseNumberOfSentDatagrams(), 5, nick.length() +
							opponentsSpinner.getValue().toString().length() + 1, nick + "," +
							opponentsSpinner.getValue().toString());
					udp.sendMessage(m, receiver.getSentMessages());
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
					Message m = new Message(udp.increaseNumberOfSentDatagrams(), 7, nick.length(), nick);
					udp.sendMessage(m, receiver.getSentMessages());
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

		JMenu helpMenu = new JMenu("Help");
		helpMenu.setMnemonic(KeyEvent.VK_H);
		JMenuItem about = new JMenuItem("About");
		helpMenu.add(about);

		menu.add(gameMenu);
		menu.add(helpMenu);

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

		canvas.setPreferredSize(new Dimension(getWidth(), getHeight() - statusLabel.getHeight() - guessedWordLabel
				.getHeight()));
		addComponentListener(new ComponentAdapter() {
			@Override
			public void componentResized(ComponentEvent e) {
				resizeStatusLabel();
				resizeGuessedWordLabel();
			}
		});
		statusLabel.addPropertyChangeListener("text", new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				resizeStatusLabel();
			}
		});
		guessedWordLabel.addPropertyChangeListener("text", new PropertyChangeListener() {
			@Override
			public void propertyChange(PropertyChangeEvent evt) {
				resizeGuessedWordLabel();
			}
		});
	}

	private void resizeStatusLabel() {
		if (statusLabel.getText().length() * statusLabel.getFont().getSize() > getWidth()) {
			while (statusLabel.getText().length() * statusLabel.getFont().getSize() > getWidth()) {
				statusLabel.setFont(new Font(statusLabel.getFont().getName(), Font.PLAIN, statusLabel.getFont()
						.getSize() - 1));
			}
		} else if (statusLabel.getText().length() * (statusLabel.getFont().getSize() + 1) < getWidth() * 0.9 &&
				statusLabel.getFont().getSize() + 1 < 105) {
			while (statusLabel.getText().length() * (statusLabel.getFont().getSize() + 1) < getWidth() * 0.9) {
				statusLabel.setFont(new Font(statusLabel.getFont().getName(), Font.PLAIN, statusLabel.getFont()
						.getSize() + 1));

			}
		}
	}

	private void resizeGuessedWordLabel() {
		if (guessedWordLabel.getText().length() * guessedWordLabel.getFont().getSize() > getWidth()) {
			while (guessedWordLabel.getText().length() * guessedWordLabel.getFont().getSize() > getWidth()) {
				guessedWordLabel.setFont(new Font(guessedWordLabel.getFont().getName(), Font.PLAIN, guessedWordLabel
						.getFont().getSize() - 1));
			}
		} else if (guessedWordLabel.getText().length() * (guessedWordLabel.getFont().getSize() + 1) < getWidth() * 0.9
				&& guessedWordLabel.getFont().getSize() + 1 < 105) {
			while (guessedWordLabel.getText().length() * (guessedWordLabel.getFont().getSize() + 1) < getWidth() *
					0.9) {
				guessedWordLabel.setFont(new Font(guessedWordLabel.getFont().getName(), Font.PLAIN, guessedWordLabel
						.getFont().getSize() + 1));

			}
		}
	}

	/**
	 * Registers keyboard listener
	 */
	private void registerListeners() {
		addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				if (game != null && game.isMyMove() && ((e.getKeyChar() >= 'a' && e.getKeyChar() <= 'z') || e
						.getKeyChar() == '\'' || e.getKeyChar() == ' ')) {
					String data = e.getKeyChar() + "";
					Message m = new Message(udp.increaseNumberOfSentDatagrams(), 18, nick.length() + 1 + data.length()
							, nick + "," + data.toUpperCase());
					udp.sendMessage(m, receiver.getSentMessages());
					lastGuessed = data.toUpperCase().charAt(0);
					setStatusLabelText("You guessed " + lastGuessed);
					game.setMyMove(false);
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
	 * Getter for last guessed letter
	 *
	 * @return last guessed letter
	 */
	public char getLastGuessed() {
		return lastGuessed;
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
