import java.util.LinkedList;
import java.util.Random;

public class App {
    public static class Processor {
        LinkedList<Integer> list = new LinkedList<>();
        Random generator = new Random();
        final int listCapacity = 10;
        int valueCount = 0;

        public void produce() throws InterruptedException {
            while (true) {
                synchronized (this) {
                    while (list.size() == listCapacity)
                        wait();

                    int value = valueCount++;
                    list.add(value);
                    System.out.println("Produced: " + value);

                    notifyAll();
                }
            }
        }

        public void consume() throws InterruptedException {
            while (true) {
                synchronized (this) {
                    while (list.isEmpty())
                        wait();

                    int value = list.removeFirst();
                    System.out.println("Consumed: " + value);
                    notifyAll();
                }

                Thread.sleep(generator.nextInt(100));
            }
        }
    }

    public static void main(String[] args) {
        Processor processor = new Processor();
        int threadCount = Integer.parseInt(args[0]);
        System.out.println("Thread count: " + threadCount);

        for (int i = 0; i < threadCount; ++i) {
            Thread thread;

            if (i % 2 == 0) {
                thread = new Thread(() -> {
                    try {
                        processor.consume();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                });
            } else {
                thread = new Thread(() -> {
                    try {
                        processor.produce();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                });
            }

            thread.start();
        }
        try {
            Thread.sleep(30000);
        } catch (InterruptedException e) {
        }

        System.exit(0);
    }
}
