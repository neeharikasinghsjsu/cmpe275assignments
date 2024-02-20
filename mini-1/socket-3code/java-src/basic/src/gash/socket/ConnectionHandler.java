package gash.socket;

import java.net.Socket;

public interface ConnectionHandler {
    void handleConnection(Socket connection);
    void onConnectionClose(Socket connection);

}
