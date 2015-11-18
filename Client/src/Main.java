package src;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.layout.AnchorPane;
import javafx.stage.Stage;

/**
 * Main class of the game
 *
 * @author Jaroslav Klaus
 */
public class Main extends Application {

	/******************************************************************************************************************/

	/**
	 * Main entry point of the game
	 *
	 * @param args command line arguments
	 */
	public static void main(String[] args) {
		launch(args);
	}

	/**
	 * Launching method to show the game window
	 *
	 * @param primaryStage primary stage
	 */
	@Override
	public void start(Stage primaryStage) {
		try {
			primaryStage.setTitle("Hangman");

			FXMLLoader loader = new FXMLLoader(getClass().getResource("Window.fxml"));
			AnchorPane root = loader.load();
			WindowController cntrlr = loader.<WindowController>getController();
			cntrlr.setHost(getParameters().getRaw().get(0));
			cntrlr.setPort(Integer.parseInt(getParameters().getRaw().get(1)));
			int width = 800;
			int height = 600;
			Scene scene = new Scene(root, width, height);
			primaryStage.setScene(scene);
			primaryStage.show();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
