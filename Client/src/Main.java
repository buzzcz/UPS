import javax.swing.*;
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
					Window window = new Window(args[0], Integer.parseInt(args[1]));
					window.setVisible(true);
				}
			});
		} catch (Exception e) {
			JOptionPane.showMessageDialog(null, e.getMessage(), e.getClass().getSimpleName(), JOptionPane
					.ERROR_MESSAGE);
		}
	}
}
