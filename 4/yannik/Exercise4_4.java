public class Exercise4_4 {
    public static class MyThread extends Thread {
        @Override
        public void run() {
            System.out.println("Thread-ID: " + threadId());
        }
    }

    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java Exercise4_4 <n>");
            return;
        }

        Integer n = Integer.parseInt(args[0]);

        Thread[] threads = new Thread[n];
        for (int i = 0; i < n; ++i) {
            threads[i] = new MyThread();
            threads[i].start();
        }

        for (int i = 0; i < n; ++i) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                System.out.println(e.toString());
            }
        }

        System.out.println("Main started " + n + " threads.");
    }
}