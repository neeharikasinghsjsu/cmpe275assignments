package gash.socket;

import java.net.Socket;
import java.util.concurrent.PriorityBlockingQueue;

public class PriorityBlockingQueueConnectionHandler implements ConnectionHandler {
    private static final int MAX_CONNECTION  = 2048;

    private final SessionEventsDispatcher sessionEventsDispatcher;
    PriorityBlockingQueue<SessionHandler> priorityBlockingQueue = new PriorityBlockingQueue<>(
        MAX_CONNECTION,
        (sh1, sh2) -> (int)(sh1.getLastReadTime() - sh2.getLastReadTime())
    );


    public PriorityBlockingQueueConnectionHandler() {
        sessionEventsDispatcher = new SessionEventsDispatcher(this);
    }

    public void handleConnection(Socket connection) {
        var sh = new SessionHandler(connection, sessionEventsDispatcher);
        if (priorityBlockingQueue.size() > MAX_CONNECTION) {
            SessionHandler removedSessionHandler = priorityBlockingQueue.peek();
            try {
                removedSessionHandler.stopSessionSafely();
            } catch (Exception ex) {
                removedSessionHandler.stopSession();
            }
        }
        priorityBlockingQueue.add(sh);
        sh.start();
    }

    public void onConnectionClose(Socket connection) {
        priorityBlockingQueue.removeIf(sh -> sh.getConnection() == connection);
    }
}
