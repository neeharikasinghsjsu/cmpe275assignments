package gash.socket;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Iterator;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import gash.payload.BasicBuilder;
import gash.payload.Message;

public class FixNumberThreadsConnectionHandler implements ConnectionHandler {

   private static final int NUM_WORKER_THREADS = 2048;

    private final ExecutorService connectionService;
    private final ExecutorService workerService; 
    ConcurrentLinkedQueue<Socket> connectionQueue = new ConcurrentLinkedQueue<>();



    public FixNumberThreadsConnectionHandler() {
        connectionService = Executors.newSingleThreadExecutor();
        workerService = Executors.newFixedThreadPool(NUM_WORKER_THREADS);
        connectionService.submit(() -> {
            foreverLoop();
        });
    }

    private void foreverLoop() {
        while ( true) {
            Iterator<Socket> it = connectionQueue.iterator();
            while (it.hasNext()) {
                Socket conn = it.next();
                try {
                    if (conn.getInputStream().available() > 0) {
                        
                        workerService.submit(() -> runSession(conn));
                    } 
                } catch(IOException ex) {
                    forceCloseConnection(conn);
                } 
            }
            
            try {
                Thread.sleep(1);
            } catch(InterruptedException ex) {
                // ignore
            }
        }
        
    }

    

    private void runSession(Socket connection) {
		try {
			connection.setSoTimeout(10);
			var in = new BufferedInputStream(connection.getInputStream());
            
			byte[] raw = new byte[2048];
			BasicBuilder builder = new BasicBuilder();
			int len = in.read(raw);
            if (len == 0)
                return;
            else if (len == -1)
                return;

            Message msg = builder.decode(new String(raw, 0, len).getBytes());
            System.out.println(msg);
        
        } catch (Exception ex) {
            forceCloseConnection(connection);
        }
        finally {
			try {
				System.out.flush();
			} catch (Exception re) {
				re.printStackTrace();
			}
		}
	}

    private void forceCloseConnection(Socket connection) {
        if (connection != null) {
            onConnectionClose(connection);
            try {
                connection.close();
            } catch (Exception exception) {
                // ignore connectin may already be closed
            }
        }
    }

    public void handleConnection(Socket connection) {
        connectionQueue.add(connection);
    }

    public void onConnectionClose(Socket connection) {
        connectionQueue.remove(connection);
    }
}