import java.lang.Thread;

class MyThread extends Thread {
    private int n;

    public MyThread(int n) {
        this.n = n;
    }

    public void run() {
        System.out.println("Hallo ich bin Thread " + n);
    }
}

class A4_4 {
    public static void main(String[] args) {
        int n = Integer.parseInt(args[0]);
        MyThread[] threads = new MyThread[n];
        for (int i = 0; i < n; i++) {
            threads[i] = new MyThread(i + 1);
            threads[i].start();
        }
        for (int i = 0; i < n; i++) {
            try {
                threads[i].join();
            }
            catch (Exception e) { }
        }
        System.out.println("Hallo von Main..ich have " + n + " Thread erzeugt");
    }
}