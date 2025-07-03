import java.util.concurrent.atomic.AtomicInteger;

public class Simulation {
    public enum RumorState {
        LISTENING, SHARING, LOST_INTEREST
    }

    public class Member extends Thread {
        RumorState state;
        Simulation sim;
        Member[] friends;

        public Member(Simulation sim, int num_friends) {
            this.sim = sim;
            state = RumorState.LISTENING;
            friends = new Member[num_friends];
        }

        public void run() {
            synchronized(this) {
                // wait until this member knows the rumor
                while (this.state == RumorState.LISTENING && sim.sharing.get() > 0) {
                    try {
                        wait();
                    } catch (Exception e) {
                        System.out.println(e);
                    }
                }

                if (sim.sharing.get() == 0) {
                    // no member is sharing the rumor anymore
                    // notify all friends then stops the thread
                    for (Member friend : friends) {
                        synchronized(friend) {
                            friend.notify();
                        }
                    }
                    return;
                }
            }

            // now this member knows the rumor and tells its friends
            for (Member friend : friends) {
                synchronized(friend) {
                    if (friend.state == RumorState.LISTENING) {
                        sim.sharing.incrementAndGet();
                        friend.state = RumorState.SHARING;
                        friend.notifyAll();
                    }
                }
            }
            synchronized(this) {
                // now the member is not interested in the rumor for 5 seconds
                state = RumorState.LOST_INTEREST;
            }

            if (sim.sharing.decrementAndGet() == 0) {
                System.out.println("No member is sharing the rumor anymore");
                // no member is sharing the rumor anymore
                // notify all friends then stops the thread
                for (Member friend : friends) {
                    synchronized(friend) {
                        friend.notifyAll();
                    }
                }
                return;
            }

            try {
                Thread.sleep(5000);
            } catch (Exception e) {
                System.out.println(e);
            }

            // the member is interested again
            synchronized(this) {
                state = RumorState.LISTENING;
            }
        }
    }

    public Member[] members;
    public int n;
    public volatile AtomicInteger sharing;

    public Simulation(int n) {
        this.n = n;

        members = new Member[n * n];

        // create members for this simulation
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                members[i * n + j] = new Member(this, 4);
            }
        }

        // set the friends (Torus)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int up = ((i - 1 + n) % n) * n + j;
                int left = i * n + (j - 1 + n) % n;
                int right = i * n + (j + 1) % n;
                int down = (i + 1) % n + j;

                members[i * n + j].friends[0] = members[up];
                members[i * n + j].friends[1] = members[left];
                members[i * n + j].friends[2] = members[right];
                members[i * n + j].friends[3] = members[down];
            }
        }
    }

    public void begin() {
        // someone starts the rumor
        int start = (int) Math.floor(Math.random() * (n * n));
        members[start].state = RumorState.SHARING;
        sharing = new AtomicInteger(1);

        // start the member threads
        for (Member member : members) {
            member.start();
        }

        // wait for all members to finish
        for (Member member : members) {
            try {
                member.join();
            } catch (Exception e) {
                System.out.println(e);
            }
        }
    }

    public static void main(String[] args) {
        int n = 10; // 10 x 10 Torus
        Simulation sim = new Simulation(n);
        sim.begin();
    }
}