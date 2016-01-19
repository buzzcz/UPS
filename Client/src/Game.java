/**
 * @author Jaroslav Klaus
 */
public class Game {
	/**
	 * State of a game
	 * 0: waiting for opponent
	 * 1: in progress
	 * 2: waiting for disconnected player
	 */
	private int state;
	/**
	 * Number of players supposed to by in a game
	 */
	private int playersCount;
	/**
	 * Number of letter in guessed word
	 */
	private int guessedWord;
	/**
	 * Guessed letters
	 */
	private String guessedLetters;
	/**
	 * Number of wrong guesses
	 */
	private int wrongGuesses;
	/**
	 * If true then it's my move otherwise it's not
	 */
	private boolean myMove;
	/**
	 * Last sent message
	 */
	private Message lastMessage;

	/******************************************************************************************************************/

	/**
	 * Constructor for a game
	 *
	 * @param playersCount number of players supposed to be in the game
	 */
	public Game(int playersCount) {
		this.playersCount = playersCount;
		state = 0;
		guessedLetters = "";
		myMove = false;
		lastMessage = null;
	}

	public Game(int state, int playersCount, int guessedWord, String guessedLetters, int wrongGuesses) {
		this.state = state;
		this.playersCount = playersCount;
		this.guessedWord = guessedWord;
		this.guessedLetters = guessedLetters;
		this.wrongGuesses = wrongGuesses;
		myMove = false;
		lastMessage = null;
	}


	/**
	 * Getter for state
	 *
	 * @return state of the game
	 */
	public int getState() {
		return state;
	}

	/**
	 * Setter for a state
	 *
	 * @param state state of the game
	 */
	public void setState(int state) {
		this.state = state;
	}

	/**
	 * Getter for a players count
	 *
	 * @return number of players supposed to be in the game
	 */
	public int getPlayersCount() {
		return playersCount;
	}

	/**
	 * Setter for a players count
	 *
	 * @param playersCount number of players supposed to be in the game
	 */
	public void setPlayersCount(int playersCount) {
		this.playersCount = playersCount;
	}

	/**
	 * Getter for number of letters of guessed word
	 *
	 * @return number of letters of guessed word
	 */
	public int getGuessedWord() {
		return guessedWord;
	}

	/**
	 * Setter for number of letters of guessed word
	 *
	 * @param guessedWord number of letters of guessed word
	 */
	public void setGuessedWord(int guessedWord) {
		this.guessedWord = guessedWord;
	}

	/**
	 * Getter for already guessed letters
	 *
	 * @return already guessed letters
	 */
	public String getGuessedLetters() {
		return guessedLetters;
	}

	/**
	 * Getter for last sent message
	 *
	 * @return last sent message
	 */
	public Message getLastMessage() {
		return lastMessage;
	}

	/**
	 * Setter for last sent message
	 *
	 * @param lastMessage last sent message
	 */
	public void setLastMessage(Message lastMessage) {
		this.lastMessage = lastMessage;
	}

	/**
	 * Adds newly guessed letter to already guessed letters
	 *
	 * @param guessedLetter newly guessed letter
	 */
	public void addGuessedLetter(char guessedLetter) {
		if (!guessedLetters.contains(guessedLetter + "")) {
			guessedLetters += guessedLetter;
		}
	}

	/**
	 * Says if it is my move
	 *
	 * @return <code>true</code> if it is my move;<code>false</code> otherwise
	 */
	public boolean isMyMove() {
		return myMove;
	}

	/**
	 * Setter for my move
	 *
	 * @param myMove my move
	 */
	public void setMyMove(boolean myMove) {
		this.myMove = myMove;
	}

	/**
	 * Adds wrong guess to the current ones
	 */
	public void wrongGuess() {
		wrongGuesses++;
	}
}
