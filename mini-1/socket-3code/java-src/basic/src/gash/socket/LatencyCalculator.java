package gash.socket;

public class LatencyCalculator {
    private final long startMillis;
    private long endMillis = 0;

    public LatencyCalculator() {
        startMillis = System.currentTimeMillis();
    }

    public void end() {
        endMillis = System.currentTimeMillis();
    }

    public long latency() {
        if (endMillis == 0) {
            throw new RuntimeException("End time is not set for latency calculation");
        }
        return endMillis - startMillis;
    }

}
