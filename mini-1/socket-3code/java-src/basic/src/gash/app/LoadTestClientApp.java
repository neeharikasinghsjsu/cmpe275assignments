package gash.app;

import java.util.ArrayList;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import gash.socket.BasicClient;
import gash.socket.LatencyCalculator;
import gash.socket.ReportService;

public class LoadTestClientApp {

    private static BasicClient newConnectedClient() {
        var myClient = new BasicClient("app", "127.0.0.1", 2000);
		myClient.connect();
		myClient.join("pets/dogs");

        return myClient;
    }
    
	public static void main(String[] args) {
        int numClients =  Integer.valueOf(args[0]);
        int perClientQPS = Integer.valueOf(args[1]);
        String reportSavePath = args[2];

        ReportService reportService = new ReportService();
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            reportService.stopAndSave(reportSavePath);
        }));

        int numConnected = 0;
        int numFailed = 0;
        ArrayList<BasicClient> connectedClients = new ArrayList<>();
        for (int i = 0; i < numClients; i++) {
            var myClient = newConnectedClient();
            if (myClient.isConnectedToServer()) {
                numConnected ++;
                connectedClients.add(myClient);
            } else {
                numFailed++;
            } 
        }

        reportService.addToCustomReport("numConnected", numConnected);
        reportService.addToCustomReport("numFailed", numFailed);

        // Create a ScheduledExecutorService with the desired number of threads
        ScheduledExecutorService executorService = Executors.newScheduledThreadPool(Math.min(connectedClients.size() - 1, 100));
        ArrayList<Runnable> tasks = new ArrayList<>();

        for (BasicClient client: connectedClients) {
            tasks.add(createTaskForClient(client, reportService));
        }

        for (int i = 0; i < tasks.size(); ++i) {
            executorService.scheduleAtFixedRate(tasks.get(i), i, (int)Math.ceil(1000.0/perClientQPS), TimeUnit.MILLISECONDS);
        }
    }

    private static Runnable createTaskForClient(BasicClient client, ReportService reportService) {
        return () -> {
            LatencyCalculator latencyCalculator = new LatencyCalculator();
            client.sendMessage("simple sample message from client " + client.getClientId(), 5);
            latencyCalculator.end();
            reportService.addLatencyReport("client-sent", latencyCalculator);
            reportService.addToRequestCounter();
        };
    }
}