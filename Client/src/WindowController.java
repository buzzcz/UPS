package src;

import javafx.application.Platform;
import javafx.beans.property.DoubleProperty;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.geometry.Point2D;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.AnchorPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;

import java.net.URL;
import java.util.ResourceBundle;

/**
 * Class for controlling the windows
 *
 * @author Jaroslav Klaus
 */
public class WindowController implements Initializable {
	/**
	 * Root
	 */
	@FXML
	private AnchorPane root;
	/**
	 * VBox with canvas
	 */
	@FXML
	private VBox canvasVBox;
	/**
	 * Headline label
	 */
	@FXML
	private Label statusLabel;
	/**
	 * Font size property for headline label
	 */
	private DoubleProperty fontSize;
	/**
	 * Canvas for painting the hangman
	 */
	@FXML
	private Canvas canvas;
	/**
	 * VBox with keys
	 */
	@FXML
	private VBox keyboardVBox;
	/**
	 * Keyboard buttons
	 */
	@FXML
	private Button qButton, wButton, eButton, rButton, tButton, yButton, uButton, iButton, oButton, pButton, aButton,
			sButton, dButton, fButton, gButton, hButton, jButton, kButton, lButton, zButton, xButton, cButton,
			vButton, bButton, nButton, mButton, spaceButton;
	/**
	 * Label with connection
	 */
	@FXML
	private Label connectionLabel;
	/**
	 * Label with user
	 */
	@FXML
	private Label userLabel;

	/**
	 * Connection
	 */
	private Connection udp;
	private String host;
	private int port;

	/**
	 * Graphics context
	 */
	private GraphicsContext g;
	/**
	 * Coordinates of the graphics context
	 */
	private Point2D coord;

	/**
	 * Number of wrong guessed letters
	 */
	private int missedLetters;

	/******************************************************************************************************************/

	/**
	 * First method automatically ran when this window is loaded
	 *
	 * @param location  location
	 * @param resources resources
	 */
	@Override
	public void initialize(URL location, ResourceBundle resources) {
		fontSize = new SimpleDoubleProperty(20);
		g = canvas.getGraphicsContext2D();
		bindSizes();
		addListeners();
		addKeyboardButtonActions();

		missedLetters = 11;

		Platform.runLater(() -> {
			udp = new Connection(host, port);
			root.getScene().getWindow().setOnCloseRequest(event -> {
				exitGame();
				event.consume();
			});
			addKeybordEventFilters();
			statusLabel.setFont(Font.font(statusLabel.getFont().getFamily(), fontSize.doubleValue()));
			draw();
		});
	}

	/**
	 * Method that closes the socket and exits the game
	 */
	@FXML
	public void exitGame() {
		if (udp != null) udp.close();
		Platform.exit();
		System.exit(0);
	}

	/**
	 * Binds sizes of containers to other elements
	 */
	private void bindSizes() {
		keyboardVBox.minHeightProperty().bind(root.heightProperty().multiply(0.3));
		canvasVBox.minHeightProperty().bind(root.heightProperty().multiply(0.6));
		canvasVBox.minWidthProperty().bind(root.widthProperty());
		canvas.heightProperty().bind(canvasVBox.heightProperty().multiply(0.9));
		canvas.widthProperty().bind(canvasVBox.widthProperty());
		fontSize.bind(canvasVBox.heightProperty().multiply(0.1));
	}

	/**
	 * Adds listeners to containers and elements for certain actions
	 */
	private void addListeners() {
		canvas.heightProperty().addListener(observable -> {
			statusLabel.setFont(Font.font(statusLabel.getFont().getFamily(), fontSize.doubleValue()));
			draw();
		});
		canvas.widthProperty().addListener(observable -> draw());
	}

	/**
	 * Method that defines what should happen if the virtual keyboard keys are clicked
	 */
	private void addKeyboardButtonActions() {
//		TODO create actions
		qButton.setOnAction(event -> {
			String data = "Q";
			udp.sendMessage(new Message(udp.getNumberOfSentDatagrams() + ";17;" + data.length() + ";" + data));
			System.out.println(udp.receiveMessage().getMessage());
		});
		wButton.setOnAction(event -> {

		});
		eButton.setOnAction(event -> {

		});
		rButton.setOnAction(event -> {

		});
		tButton.setOnAction(event -> {

		});
		yButton.setOnAction(event -> {

		});
		uButton.setOnAction(event -> {

		});
		iButton.setOnAction(event -> {

		});
		oButton.setOnAction(event -> {

		});
		pButton.setOnAction(event -> {

		});
		aButton.setOnAction(event -> {

		});
		sButton.setOnAction(event -> {

		});
		dButton.setOnAction(event -> {

		});
		fButton.setOnAction(event -> {

		});
		gButton.setOnAction(event -> {

		});
		hButton.setOnAction(event -> {

		});
		jButton.setOnAction(event -> {

		});
		kButton.setOnAction(event -> {

		});
		lButton.setOnAction(event -> {

		});
		zButton.setOnAction(event -> {

		});
		xButton.setOnAction(event -> {

		});
		cButton.setOnAction(event -> {

		});
		vButton.setOnAction(event -> {

		});
		bButton.setOnAction(event -> {

		});
		nButton.setOnAction(event -> {

		});
		mButton.setOnAction(event -> {

		});
		spaceButton.setOnAction(event -> {

		});
	}

	/**
	 * Adds event filters to real keyboard and maps the action to the action of virtual keyboard
	 */
	private void addKeybordEventFilters() {
		root.getScene().addEventFilter(KeyEvent.KEY_RELEASED, event -> {
			if (!event.isShortcutDown() && !event.isAltDown()) {
				switch (event.getCode()) {
					case Q:
						qButton.fire();
						event.consume();
						break;
					case W:
						wButton.fire();
						event.consume();
						break;
					case E:
						eButton.fire();
						event.consume();
						break;
					case R:
						rButton.fire();
						event.consume();
						break;
					case T:
						tButton.fire();
						event.consume();
						break;
					case Y:
						yButton.fire();
						event.consume();
						break;
					case U:
						uButton.fire();
						event.consume();
						break;
					case I:
						iButton.fire();
						event.consume();
						break;
					case O:
						oButton.fire();
						event.consume();
						break;
					case P:
						pButton.fire();
						event.consume();
						break;

					case A:
						aButton.fire();
						event.consume();
						break;

					case S:
						sButton.fire();
						event.consume();
						break;

					case D:
						dButton.fire();
						event.consume();
						break;

					case F:
						fButton.fire();
						event.consume();
						break;

					case G:
						gButton.fire();
						event.consume();
						break;

					case H:
						hButton.fire();
						event.consume();
						break;

					case J:
						jButton.fire();
						event.consume();
						break;

					case K:
						kButton.fire();
						event.consume();
						break;

					case L:
						lButton.fire();
						event.consume();
						break;
					case Z:
						zButton.fire();
						event.consume();
						break;
					case X:
						xButton.fire();
						event.consume();
						break;
					case C:
						cButton.fire();
						event.consume();
						break;
					case V:
						vButton.fire();
						event.consume();
						break;
					case B:
						bButton.fire();
						event.consume();
						break;
					case N:
						nButton.fire();
						event.consume();
						break;
					case M:
						mButton.fire();
						event.consume();
						break;
					case SPACE:
						spaceButton.fire();
						event.consume();
						break;
				}
			}
		});
	}

	/**
	 * Redraws a hangman according to a number of wrong guessed letters
	 */
	private void draw() {
		g.clearRect(0, 0, canvas.getWidth(), canvas.getHeight());
		switch (missedLetters) {
			case 1:
				drawFirst();
				break;
			case 2:
				drawSecond();
				break;
			case 3:
				drawThird();
				break;
			case 4:
				drawFourth();
				break;
			case 5:
				drawFifth();
				break;
			case 6:
				drawSixth();
				break;
			case 7:
				drawSeventh();
				break;
			case 8:
				drawEighth();
				break;
			case 9:
				drawNinth();
				break;
			case 10:
				drawTenth();
				break;
			case 11:
				drawEleventh();
				break;
		}
	}

	/**
	 * Draws a hangman for one wrong guessed letter
	 */
	private void drawFirst() {
		g.beginPath();
		g.setStroke(Color.BLACK);
		g.setLineWidth(3);

		coord = new Point2D(canvas.getWidth() / 2, 4 * canvas.getHeight() / 5);
		g.moveTo(coord.getX() - canvas.getWidth() / 15, coord.getY());
		g.lineTo(coord.getX() + canvas.getWidth() / 15, coord.getY());
		g.stroke();
	}

	/**
	 * Draws a hangman for two wrong guessed letters
	 */
	private void drawSecond() {
		drawFirst();

		g.moveTo(coord.getX(), coord.getY());
		coord = coord.add(0, -canvas.getHeight() / 2);
		g.lineTo(coord.getX(), coord.getY());
		g.stroke();
	}

	/**
	 * Draws a hangman for three wrong guessed letters
	 */
	private void drawThird() {
		drawSecond();

		coord = coord.add(0, canvas.getHeight() / 10);
		g.moveTo(coord.getX(), coord.getY());
		coord = coord.add(-canvas.getWidth() / 15, -canvas.getHeight() / 10);
		g.lineTo(coord.getX(), coord.getY());
		g.stroke();
	}

	/**
	 * Draws a hangman for four wrong guessed letters
	 */
	private void drawFourth() {
		drawThird();

		coord = coord.add(canvas.getWidth() / 15, 0);
		g.moveTo(coord.getX(), coord.getY());
		coord = coord.add(-canvas.getWidth() / 10, 0);
		g.lineTo(coord.getX(), coord.getY());
		g.stroke();
	}

	/**
	 * Draws a hangman for five wrong guessed letters
	 */
	private void drawFifth() {
		drawFourth();

		g.moveTo(coord.getX(), coord.getY());
		coord = coord.add(0, canvas.getHeight() / 10);
		g.lineTo(coord.getX(), coord.getY());
		g.stroke();
	}

	/**
	 * Draws a hangman for six wrong guessed letters
	 */
	private void drawSixth() {
		drawFifth();

		g.setFill(Color.BLACK);

		g.moveTo(coord.getX(), coord.getY());
		g.fillOval(coord.getX() - canvas.getWidth() / 30, coord.getY() - canvas.getWidth() / 50, canvas.getWidth() /
				25, canvas.getWidth() / 25);
		coord = coord.add(0, canvas.getWidth() / 65);
	}

	/**
	 * Draws a hangman for seven wrong guessed letters
	 */
	private void drawSeventh() {
		drawSixth();

		g.beginPath();
		g.setLineWidth(8);

		g.moveTo(coord.getX(), coord.getY());
		g.lineTo(coord.getX(), coord.getY() + canvas.getHeight() / 10);
		g.stroke();
	}

	/**
	 * Draws a hangman for eight wrong guessed letters
	 */
	private void drawEighth() {
		drawSeventh();

		g.moveTo(coord.getX(), coord.getY());
		g.lineTo(coord.getX() - canvas.getWidth() / 50, coord.getY() + canvas.getHeight() / 10);
		g.stroke();
	}

	/**
	 * Draws a hangman for nine wrong guessed letters
	 */
	private void drawNinth() {
		drawEighth();

		g.moveTo(coord.getX(), coord.getY());
		g.lineTo(coord.getX() + canvas.getWidth() / 50, coord.getY() + canvas.getHeight() / 10);
		g.stroke();
	}

	/**
	 * Draws a hangman for ten wrong guessed letters
	 */
	private void drawTenth() {
		drawNinth();

		coord = coord.add(0, canvas.getHeight() / 10);
		g.moveTo(coord.getX(), coord.getY());
		g.lineTo(coord.getX() - canvas.getWidth() / 50, coord.getY() + canvas.getHeight() / 10);
		g.stroke();
	}

	/**
	 * Draws a hangman for eleven wrong guessed letters. This is a complete hangman
	 */
	private void drawEleventh() {
		drawTenth();

		g.moveTo(coord.getX(), coord.getY());
		g.lineTo(coord.getX() + canvas.getWidth() / 50, coord.getY() + canvas.getHeight() / 10);
		g.stroke();
	}

	/**
	 * Getter for number of wrong guessed letters
	 *
	 * @return number of wrong guessed letters
	 */
	public int getMissedLetters() {
		return missedLetters;
	}


	public void setHost(String host) {
		this.host = host;
	}

	public void setPort(int port) {
		this.port = port;
	}
}
