import java.awt.*;

/**
 * @author Jaroslav Klaus
 */
public class Main {

	public static void main(final String[] args) {
		try {
			EventQueue.invokeLater(new Runnable() {
				@Override
				public void run() {
					try {
						Window window = new Window();
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
