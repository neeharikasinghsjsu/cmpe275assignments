package gash.socket;

import java.io.BufferedInputStream;
import java.io.InterruptedIOException;
import java.net.Socket;

import gash.payload.BasicBuilder;
import gash.payload.Message;


/**
 * 
 * @author gash
 * 
 */
class SessionHandler extends Thread {
	private Socket connection;
	private boolean forever = true;
    private long lastReadTime = 0;

    private final SessionEventsDispatcher sessionEventsDispatcher;

	public SessionHandler(Socket connection, SessionEventsDispatcher sessionEventsDispatcher) {
		this.connection = connection;
		this.sessionEventsDispatcher = sessionEventsDispatcher;
		// allow server to exit if
		this.setDaemon(true);
	}

    public Socket getConnection() {
        return connection;
    }

	public void stopSessionSafely() throws Exception {
		forever = false;
        while (connection.getInputStream().available() > 0) {
            Thread.sleep(10);
        }
        stopSession();
	}

	/**
	 * stops session on next timeout cycle
	 */
	public void stopSession() {
		forever = false;
		if (connection != null) {
			try {
                sessionEventsDispatcher.onConnectionClose(connection);
				connection.close();  
			} catch (Exception e) {
				//e.printStackTrace();
			}
		}
		connection = null;
	}

	/**
	 * process incoming data
	 */
	public void run() {
		System.out.println("Session " + this.getId() + " started");

		try {
			connection.setSoTimeout(2000);
			var in = new BufferedInputStream(connection.getInputStream());
            
			byte[] raw = new byte[2048];
			BasicBuilder builder = new BasicBuilder();
			while (forever) {
				try {
					int len = in.read(raw);
					if (len == 0)
						continue;
					else if (len == -1)
						break;

					Message msg = builder.decode(new String(raw, 0, len).getBytes());
					System.out.println(msg);
                    lastReadTime = System.currentTimeMillis();
				} catch (InterruptedIOException ioe) {
				}
			}
		} catch (Exception e) {
            if (forever != false) {
			    e.printStackTrace();
            }

		} finally {
			try {
				System.out.println("Session " + this.threadId() + " ending");
				System.out.flush();
				stopSession();
			} catch (Exception re) {
				re.printStackTrace();
			}
		}
	}

    public long getLastReadTime() {
        return lastReadTime;
    }

} // class SessionHandler