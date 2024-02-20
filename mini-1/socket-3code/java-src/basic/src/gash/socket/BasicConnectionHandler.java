package gash.socket;

import java.net.Socket;

public class BasicConnectionHandler implements ConnectionHandler {
    private final SessionEventsDispatcher sessionEventsDispatcher;

    public BasicConnectionHandler() {
        sessionEventsDispatcher = new SessionEventsDispatcher(this);
    }

    public void handleConnection(Socket connection) {
        var sh = new SessionHandler(connection, sessionEventsDispatcher);
        sh.start();
    }

    public void onConnectionClose(Socket connection) {
        // do nothing for BasicConnection Handling
    }
}
