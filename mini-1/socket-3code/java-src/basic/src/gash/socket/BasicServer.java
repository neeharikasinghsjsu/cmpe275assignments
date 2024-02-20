package gash.socket;

import java.net.ServerSocket;
import java.net.Socket;

/**
 * server to manage incoming clients
 * 
 * @author gash
 * 
 */
public class BasicServer {
	private String host;
	private int port;
	private ServerSocket socket;
	private boolean forever = true;

    private ReportService serverReport;
	
	public BasicServer(String host, int port) {
		this.host = host;
		this.port = port;
        serverReport = new ReportService();
	}

	/**
	 * start monitoring socket for new connections
	 */
	public void start(ConnectionHandler connectionHandler) {
		try {
			socket = new ServerSocket(port);

			System.out.println("Server Host: " + socket.getInetAddress().getHostAddress());

			while (forever) {
				Socket s = socket.accept();
				if (!forever) {
					break;
				}

				System.out.println("--> server got a client connection");
                
                LatencyCalculator latencyCalculator = new LatencyCalculator();
                connectionHandler.handleConnection(s);
                latencyCalculator.end();
                serverReport.addLatencyReport("connection", latencyCalculator);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
        if (socket != null) {
            try {
                socket.close();
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
	}

    public ReportService getServerReport() {
        return serverReport;
    }
}
