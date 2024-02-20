package gash.socket;

import java.net.Socket;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ConcurrentQueueConnectionHandler implements ConnectionHandler {
    private static final int MAX_CONNECTION  = 2048;

    private final SessionEventsDispatcher sessionEventsDispatcher;
    ConcurrentLinkedQueue<SessionHandler> concurrentQueue = new ConcurrentLinkedQueue<>();


    public ConcurrentQueueConnectionHandler() {
        sessionEventsDispatcher = new SessionEventsDispatcher(this);
    }

    public void handleConnection(Socket connection) {
        var sh = new SessionHandler(connection, sessionEventsDispatcher);
        if (concurrentQueue.size() >= MAX_CONNECTION) {
            SessionHandler removedSessionHandler = concurrentQueue.peek();
            try {
                removedSessionHandler.stopSessionSafely();
            } catch (Exception ex) {
                removedSessionHandler.stopSession();
            }
        }
        concurrentQueue.add(sh);
        sh.start();
    }

    public void onConnectionClose(Socket connection) {
        concurrentQueue.removeIf(sh -> sh.getConnection() == connection);
    }
}
