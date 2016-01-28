import java.awt.*;

/**
 * @author Jaroslav Klaus
 */
public class Main {

	public static void main(final String[] args) {
		if (args.length != 2) {
			System.out.println("Client must be started with 2 parameters: 1) server's address, 2) server's port");
			System.exit(-1);
		}
		try {
			EventQueue.invokeLater(new Runnable() {
				@Override
				public void run() {
					try {
						Window window = new Window(args[0], Integer.parseInt(args[1]));
						window.setVisible(true);
					} catch (NumberFormatException e) {
						System.out.println("Port is not a valid number");
						System.exit(-1);
					}
				}
			});
		} catch (Exception e) {
			System.out.println(e.getMessage());
			System.exit(-1);
		}
	}
}
