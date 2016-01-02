import java.awt.*;

/**
 * @author Jaroslav Klaus
 */
public class Main {

	public static void main(final String[] args) {

		EventQueue.invokeLater(new Runnable() {
			@Override
			public void run() {
				Window window = new Window(args[0], Integer.parseInt(args[1]));
				window.setVisible(true);
			}
		});
	}
}
