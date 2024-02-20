package gash.app;


import gash.socket.BasicConnectionHandler;
import gash.socket.BasicServer;
import gash.socket.ConcurrentQueueConnectionHandler;
import gash.socket.ConnectionHandler;
import gash.socket.FixNumberThreadsConnectionHandler;
import gash.socket.PriorityBlockingQueueConnectionHandler;

/**
 * server application
 * 
 * @author gash
 * 
 */
class ServerApp {
	public ServerApp() {
	}

	public static void main(String[] args) {
        String connectionHandlerType = args.length > 0 ? args[0] : "basic";
        String reportSavePath = args.length > 1 ? args[1] : null;

        ConnectionHandler connectionHandler = switch (connectionHandlerType) {
            case "basic" -> new BasicConnectionHandler();
            case "creationQ" -> new ConcurrentQueueConnectionHandler();
            case "priorityQ" -> new PriorityBlockingQueueConnectionHandler();
            case "fixedWorkers" -> new FixNumberThreadsConnectionHandler();
            default -> new BasicConnectionHandler();
        };

        var host = "127.0.0.1";
		var port = 2000;
		var server = new BasicServer(host,port);

        System.out.println("reportSavePath:" + reportSavePath);
        if (reportSavePath != null) {
            System.out.println("registering shutdown hook"); 
            Runtime.getRuntime().addShutdownHook(new Thread(() -> {
                System.out.println("on shutdown");
                server.getServerReport().stopAndSave(reportSavePath);
            }));
        }

        server.start(connectionHandler);
	}
}
