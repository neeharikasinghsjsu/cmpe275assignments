package gash.socket;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.stream.Collectors;

public class ReportService {

    private final ExecutorService executorService;

    private HashMap<String, ArrayList<Long>> latencies = new HashMap<>();
    private HashMap<Integer, Integer> requestCounterPerMinute = new HashMap<>();
    private HashMap<String, Integer> customReport = new HashMap<>();
    


    public ReportService() {
        executorService = Executors.newSingleThreadExecutor();
    }
    
    public void addLatencyReport(String type, LatencyCalculator latencyCalculator) {
        executorService.submit(() -> {
            if (!latencies.containsKey(type)) {
                latencies.put(type, new ArrayList<>());
            }
            latencies.get(type).add(latencyCalculator.latency());
        });
    }

    public void addToRequestCounter() {
        executorService.submit(() -> {
            int minute = (int)(System.currentTimeMillis()/(1000 * 60));
            
            requestCounterPerMinute.put(minute,
                1 + requestCounterPerMinute.getOrDefault(minute, 0));
        });
    }

    public void addToCustomReport(String key, int value) {
        customReport.put(key, value);
    }

    public void stopAndSave(String filePath) {
        executorService.shutdown();
        StringBuffer sbReport = generateReport();
        writeStringBufferToFile(sbReport, filePath);
    }

    private StringBuffer generateReport() {
        StringBuffer sb = new StringBuffer();
        for(var entry: customReport.entrySet()) {
            addRowToReport(sb, entry.getKey(), entry.getValue());
        }

        if (requestCounterPerMinute.size() > 0) {
            addMaxQPSToReport(sb);
        }
       
        for(var entry: latencies.entrySet()) {
            sb.append("Latency Report for " + entry.getKey() + "\n");
            ArrayList<Long> latencyVals = entry.getValue();
            Collections.sort(latencyVals);
            addTimeRowToReport(sb, "p50", percentile(latencyVals, 50.0));
            addTimeRowToReport(sb, "p90", percentile(latencyVals, 90.0));
            addTimeRowToReport(sb, "p99", percentile(latencyVals, 99.0));
            addTimeRowToReport(sb, "avg", latencyVals.stream().collect(
                Collectors.summingDouble(Long::doubleValue))
                 / latencyVals.size());
        }
        return sb;  
    }

    private void addMaxQPSToReport(StringBuffer sb) {
        addRowToReport(sb, "max-qps", 
        Collections.max(requestCounterPerMinute.values())/60.0);
    }

    private static double percentile(List<Long> vals, double percentile) {
        return vals.get((int) Math.round(percentile / 100.0 * (vals.size() - 1)));
    }

    private void addTimeRowToReport(StringBuffer sb, String key, double val) {
        sb.append(key);
        sb.append(":");
        sb.append(String.valueOf(val));
        sb.append("ms");
        sb.append("\n");
    }

    private void addRowToReport(StringBuffer sb, String key, double val) {
        sb.append(key);
        sb.append(":");
        sb.append(String.valueOf(val));
        sb.append("\n");
    }

    private static void writeStringBufferToFile(StringBuffer stringBuffer, String filePath) {
        String content = stringBuffer.toString();
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(filePath))) {
            writer.write(content);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
