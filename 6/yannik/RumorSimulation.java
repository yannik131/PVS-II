import java.util.Random;

public class RumorSimulation {
    public static class Member {
        private Member[] friends;

        boolean receivedRumor = false;

        public static Member[] world;
        private static int rumorCount = 0;
        private static boolean weAreDone = false;
        final private static Object lock = new Object();

        void setFriends(Member[] friends) {
            this.friends = friends;
        }

        void tellRumor() {
            receivedRumor = true;
        }

        void startListening() throws InterruptedException {
            while (true) {
                synchronized (this) {
                    receivedRumor = false;
                    while (!receivedRumor && !weAreDone)
                        wait();

                    if (weAreDone)
                        return;
                }

                synchronized (lock) {
                    ++rumorCount;
                    System.out.println("++rumorCount -> " + rumorCount);
                }

                for (int i = 0; i < friends.length; ++i) {
                    synchronized (friends[i]) {
                        friends[i].tellRumor();
                        friends[i].notify();
                    }
                }

                Thread.sleep(5000);

                synchronized (lock) {
                    --rumorCount;
                    System.out.println("--rumorCount -> " + rumorCount);

                    if (rumorCount == 0) {
                        weAreDone = true;
                        for (int i = 0; i < world.length; ++i) {
                            synchronized (world[i]) {
                                world[i].notify();
                            }
                        }
                        return;
                    }
                }
            }
        }
    }

    public static Member[] getFriends(int memberIndex, Member[] members) {
        int width = 10;
        int height = 10;

        int row = memberIndex / width;
        int col = memberIndex % width;

        int leftCol = (col - 1 + width) % width;
        int rightCol = (col + 1) % width;
        int topRow = (row - 1 + height) % height;
        int bottomRow = (row + 1) % height;

        return new Member[] { members[row * width + leftCol], members[topRow * width + col],
                members[row * width + rightCol], members[bottomRow * width + col] };
    }

    public static void main(String[] args) {
        Member[] members = new Member[100];
        for (int i = 0; i < members.length; ++i)
            members[i] = new Member();

        Member.world = members;

        for (int i = 0; i < members.length; ++i)
            members[i].setFriends(getFriends(i, members));

        Thread[] threads = new Thread[100];

        for (int i = 0; i < threads.length; ++i) {
            final int index = i;
            threads[i] = new Thread(() -> {
                try {
                    members[index].startListening();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            });
            threads[i].start();
        }

        Random random = new Random();
        int index = random.nextInt(100);

        synchronized (members[index]) {
            members[index].tellRumor();
            members[index].notify();
        }

        for (int i = 0; i < threads.length; ++i) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
