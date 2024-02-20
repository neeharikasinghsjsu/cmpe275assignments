package gash.socket;

import java.net.Socket;

public class SessionEventsDispatcher {
    private final ConnectionHandler connectionHandler;
    public SessionEventsDispatcher(ConnectionHandler ch) {
        connectionHandler = ch;
    }

    public void onConnectionClose(Socket connection) {
        connectionHandler.onConnectionClose(connection);
    }
}
