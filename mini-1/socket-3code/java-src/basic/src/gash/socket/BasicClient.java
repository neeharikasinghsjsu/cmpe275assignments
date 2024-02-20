package gash.socket;

import java.io.IOException;
import java.net.Socket;
import java.util.UUID;

import gash.payload.BasicBuilder;
import gash.payload.Message;

public class BasicClient {
	
    private String name;
	private String ipaddr;
	private int port;
	private String group = "public";
    private boolean isConnected = false;
    private String uuid;

	private Socket clt;

	public BasicClient(String name) {
		this(name, "127.0.0.1", 2000);
	}

	public BasicClient(String name, String ipaddr, int port) {
		this.name = name;
		this.ipaddr = ipaddr;
		this.port = port;
        this.uuid = UUID.randomUUID().toString();
	}

    public String getClientId() {
        return this.uuid;
    }


	public void stop() {
		stop(false);
	}

    public void stop(boolean silentFail) {
		if (this.clt != null) {
			try {
				this.clt.close();
			} catch (IOException e) {
				// TODO better error handling? yes!
                if (!silentFail) {
				    e.printStackTrace();
                }
			}
		}
		this.clt = null;
	}

	public void join(String group) {
		this.group = group;
	}

	public boolean connect() {
		if (this.clt != null) {
			return false;
		}

		try {
			this.clt = new Socket(this.ipaddr, this.port);
			System.out.println("Connected to " + clt.getInetAddress().getHostAddress());
            isConnected = true;
            return true;
		} catch (Exception e) {
			e.printStackTrace();
            return false;
		}
	}

	public void sendMessage(String message) {
        sendMessage(message, 0);
	}

    public void sendMessage(String message, int numRetries) {
		if (this.clt == null) {
			System.out.println("no connection, text not sent");
			return;
		}

		try {
			BasicBuilder builder = new BasicBuilder();
			byte[] msg = builder.encode(new Message(name, group, message)).getBytes();
			this.clt.getOutputStream().write(msg);
		} catch (Exception e) {
            isConnected = false;
            
            if (numRetries > 0) {
                //try reconnecting
                stop(true);
                boolean connected = connect();
                if (connected) {
                    sendMessage(message, numRetries - 1);
                } else {
                    e.printStackTrace();
                }
            } else {
                e.printStackTrace();
            }	
		}
	}

    public boolean isConnectedToServer() {
        return isConnected;
    }
}
