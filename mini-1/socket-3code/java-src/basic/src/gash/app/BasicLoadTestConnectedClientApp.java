package gash.app;

import gash.socket.BasicClient;

public class BasicLoadTestConnectedClientApp {


	public static void main(String[] args) {
        int numConnected = 0;
        int numFailed = 0;
        for (int i = 0; i < 10000; i++) {
            var myClient = new BasicClient("app", "127.0.0.1", 2000);
            long startMillis = System.currentTimeMillis();
		    if (myClient.connect()) {
                numConnected ++;
                System.out.println(String.format("connection latency: %d ms", System.currentTimeMillis() - startMillis));
                System.out.println(String.format("num client %d connected", numConnected));
                myClient.stop();
            } else {
                numFailed++;
            }
            
        }
        System.out.println(String.format("Connected: %d, Failed: %d", numConnected, numFailed));
	}
}