import javax.swing.*;
import java.awt.*;
import java.awt.geom.GeneralPath;

/**
 * @author Jaroslav Klaus
 */
public class Canvas extends JPanel {
	/**
	 * Graphics context
	 */
	private Graphics2D g;
	/**
	 * Coordinates of the graphics context
	 */
	private Point coord;
	/**
	 * Number of wrong guessed letters
	 */
	private int wrongGuesses;

	/******************************************************************************************************************/

	/**
	 * Constructor for canvas
	 */
	public Canvas() {
		wrongGuesses = 0;
	}

	/**
	 * Constructor for canvas
	 * @param wrongGuesses number of wrong guesses
	 */
	public Canvas(int wrongGuesses) {
		this.wrongGuesses = wrongGuesses;
	}

	/**
	 * Setter for wrong guesses
	 * @param wrongGuesses number of wrong guesses
	 */
	public void setWrongGuesses(int wrongGuesses) {
		this.wrongGuesses = wrongGuesses;
	}

	/**
	 * Adds wrong guess and repaints the canvas
	 */
	public void wrongGuess() {
		wrongGuesses++;
		repaint();
	}

	@Override
	public void paint(Graphics g1) {
		g = (Graphics2D) g1;
		draw(wrongGuesses);
	}

	/**
	 * Redraws a hangman according to a number of wrong guessed letters
	 *
	 * @param wrongGuesses number of wrong guessed letters
	 */
	public void draw(int wrongGuesses) {
		g.clearRect(0, 0, getWidth(), getHeight());
		switch (wrongGuesses) {
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
			default:
				break;
		}
	}

	/**
	 * Draws a hangman for one wrong guessed letter
	 */
	private void drawFirst() {
		GeneralPath path = new GeneralPath();
		g.setColor(Color.BLACK);
		g.setStroke(new BasicStroke(3, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER));

		coord = new Point(getWidth() / 2, 4 * getHeight() / 5);
		path.moveTo(coord.getX() - getWidth() / 8, coord.getY());
		path.lineTo(coord.getX() + getWidth() / 8, coord.getY());
		g.draw(path);
	}

	/**
	 * Draws a hangman for two wrong guessed letters
	 */
	private void drawSecond() {
		drawFirst();

		GeneralPath path = new GeneralPath();
		path.moveTo(coord.getX(), coord.getY());
		coord.translate(0, (int) (-getHeight() / 1.5));
		path.lineTo(coord.getX(), coord.getY());
		g.draw(path);
	}

	/**
	 * Draws a hangman for three wrong guessed letters
	 */
	private void drawThird() {
		drawSecond();

		GeneralPath path = new GeneralPath();
		coord.translate(0, getHeight() / 8);
		path.moveTo(coord.getX(), coord.getY());
		coord.translate(-getWidth() / 13, -getHeight() / 8);
		path.lineTo(coord.getX(), coord.getY());
		g.draw(path);
	}

	/**
	 * Draws a hangman for four wrong guessed letters
	 */
	private void drawFourth() {
		drawThird();

		GeneralPath path = new GeneralPath();
		coord.translate(getWidth() / 13, 0);
		path.moveTo(coord.getX(), coord.getY());
		coord.translate(-getWidth() / 8, 0);
		path.lineTo(coord.getX(), coord.getY());
		g.draw(path);
	}

	/**
	 * Draws a hangman for five wrong guessed letters
	 */
	private void drawFifth() {
		drawFourth();

		GeneralPath path = new GeneralPath();
		path.moveTo(coord.getX(), coord.getY());
		coord.translate(0, getHeight() / 8);
		path.lineTo(coord.getX(), coord.getY());
		g.draw(path);
	}

	/**
	 * Draws a hangman for six wrong guessed letters
	 */
	private void drawSixth() {
		drawFifth();

		g.setPaint(new GradientPaint(0, 0, Color.BLACK, 20, 20, Color.BLACK));

		GeneralPath path = new GeneralPath();
		path.moveTo(coord.getX(), coord.getY());
		g.fillOval((int) coord.getX() - getWidth() / 25, (int) coord.getY() - getWidth() / 45, getWidth() / 20,
				getWidth() / 20);
		coord.translate(0, getWidth() / 60);
	}

	/**
	 * Draws a hangman for seven wrong guessed letters
	 */
	private void drawSeventh() {
		drawSixth();

		GeneralPath path = new GeneralPath();
		g.setStroke(new BasicStroke(8, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER));

		path.moveTo(coord.getX(), coord.getY());
		path.lineTo(coord.getX(), coord.getY() + getHeight() / 8);
		g.draw(path);
	}

	/**
	 * Draws a hangman for eight wrong guessed letters
	 */
	private void drawEighth() {
		drawSeventh();

		GeneralPath path = new GeneralPath();
		path.moveTo(coord.getX(), coord.getY());
		path.lineTo(coord.getX() - getWidth() / 45, coord.getY() + getHeight() / 8);
		g.draw(path);
	}

	/**
	 * Draws a hangman for nine wrong guessed letters
	 */
	private void drawNinth() {
		drawEighth();

		GeneralPath path = new GeneralPath();
		path.moveTo(coord.getX(), coord.getY());
		path.lineTo(coord.getX() + getWidth() / 45, coord.getY() + getHeight() / 8);
		g.draw(path);
	}

	/**
	 * Draws a hangman for ten wrong guessed letters
	 */
	private void drawTenth() {
		drawNinth();

		GeneralPath path = new GeneralPath();
		coord.translate(0, getHeight() / 8);
		path.moveTo(coord.getX(), coord.getY());
		path.lineTo(coord.getX() - getWidth() / 45, coord.getY() + getHeight() / 8);
		g.draw(path);
	}

	/**
	 * Draws a hangman for eleven wrong guessed letters. This is a complete hangman
	 */
	private void drawEleventh() {
		drawTenth();

		GeneralPath path = new GeneralPath();
		path.moveTo(coord.getX(), coord.getY());
		path.lineTo(coord.getX() + getWidth() / 45, coord.getY() + getHeight() / 8);
		g.draw(path);
	}
}
