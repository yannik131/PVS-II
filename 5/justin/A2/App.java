import java.util.LinkedList;
import java.util.Random;

public class App {
    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java App <int = num processors>");
            System.exit(1);
        }

        LinkedList<Integer> list = new LinkedList<>();

        int num_threads = Integer.parseInt(args[0]);
        Thread[] threads = new Thread[num_threads];

        for (int i = 0; i < num_threads; i++) {
            threads[i] = new Processor(list, i);
            threads[i].start();
        }

        try {
            Thread.sleep(30000);
        } catch (Exception e) {
            System.out.println(e.toString());
        }

        System.exit(0);
    }
}

class Processor extends Thread {
    LinkedList<Integer> list;
    static int current_value = 0;
    int thread_number;
    boolean is_producer;

    public Processor(LinkedList<Integer> list, int thread_number) {
        this.list = list;
        this.thread_number = thread_number;
        is_producer = (thread_number % 2) == 1;
    }

    public void produce() {
        while (true) {
            _produce();
        }
    }

    private void _produce() {
        synchronized (list) {
            // if the list is full: wait
            while (list.size() >= 10) {
                try {
                    list.wait();
                } catch (Exception e) {
                    System.out.println(e.toString());
                }
            }

            // the list is not full
            list.add(current_value);
            current_value++;

            System.out.println("Thread " + thread_number + " Produced " + list.getLast());
            System.out.println(list.toString());

            list.notifyAll();
        }
    }

    public void consume() {
        Random random = new Random();
        while (true) {
            _consume();
            try {
                Thread.sleep(random.nextInt(2000));
            } catch (Exception e) {
                System.out.println(e.toString());
            }
        }
    }

    private void _consume() {
        synchronized (list) {
            while (list.size() == 0) {
                try {
                    list.wait();
                } catch (Exception e) {
                    System.out.println(e.toString());
                }
            }

            // the list is not empty
            Integer element = list.removeFirst();
            System.out.println("Thread " + thread_number + " consumed: " + element);
            System.out.println(list.toString());

            list.notifyAll();
        }
    }

    @Override
    public void run() {
        if (is_producer) {
            produce();
        }
        else {
            consume();
        }
    }
}