import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
 * @author Jaroslav Klaus
 */
public class Window extends JFrame {
	/**
	 * Connection
	 */
	private Connection udp;
	private String host;
	private int port;

	/**
	 * Canvas for painting the hangman
	 */
	private Canvas canvas;
	/**
	 * Headline label
	 */
	private JLabel statusLabel;

	/**
	 * Number of wrong guessed letters
	 */
	private int missedLetters;

	/******************************************************************************************************************/

	/**
	 * Cunstructor for a window
	 *
	 * @param host host address
	 * @param port port of socket
	 */
	public Window(String host, int port) {
		initWindow();
		this.host = host;
		this.port = port;
		udp = new Connection(host, port);

		canvas.setMissedLetters(11);
		canvas.repaint();
	}

	/**
	 * Initializes window and creates its layout and adds listeners
	 */
	private void initWindow() {
		setTitle("Hangman");
		setSize(800, 600);
		setLocationRelativeTo(null);
		setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);

		createMenu();
		createLayout();
		registerListeners();
	}

	/**
	 * Creates menu bar, menu items and actions
	 */
	private void createMenu() {
		JMenuBar menu = new JMenuBar();

		JMenu gameMenu = new JMenu("Game");
		gameMenu.setMnemonic(KeyEvent.VK_G);
		JMenuItem newGame = new JMenuItem("New Game");
		newGame.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, KeyEvent.CTRL_MASK));
		newGame.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				System.out.println("New Game");
			}
		});
		JMenuItem reconnect = new JMenuItem("Reconnect");
		reconnect.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, KeyEvent.CTRL_MASK));
		reconnect.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				System.out.println("Reconnect");
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

		addComponentListener(new ComponentAdapter() {
			@Override
			public void componentResized(ComponentEvent e) {
				statusLabel.setFont(new Font(statusLabel.getFont().getName(), Font.PLAIN, (int) (getHeight() * 0.1)));
			}
		});

		getContentPane().add(new JSeparator(SwingConstants.HORIZONTAL));

		canvas = new Canvas();
		canvas.setPreferredSize(new Dimension(getWidth(), getHeight() - statusLabel.getHeight()));
		getContentPane().add(canvas);
	}

	/**
	 * Method that closes the socket and exits the game
	 */
	public void exitClient() {
//		TODO exit game - send leaving info
		if (udp != null) udp.close();
		System.exit(0);
	}

	/**
	 * Registers keyboard listener
	 */
	private void registerListeners() {
		addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				if ((e.getKeyChar() > 'a' && e.getKeyChar() < 'z') || e.getKeyChar() == '\'' || e.getKeyChar() ==
						' ') {
					String data = e.getKeyChar() + "";
					udp.sendMessage(new Message(udp.getNumberOfSentDatagrams() + ";17;" + data.length() + ";" + data
							.toUpperCase()));
					System.out.println(udp.receiveMessage().getMessage());
				}
			}
		});
	}

	/**
	 * Getter for number of wrong guessed letters
	 *
	 * @return number of wrong guessed letters
	 */
	public int getMissedLetters() {
		return missedLetters;
	}
}
