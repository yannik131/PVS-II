import java.util.Random;

public class Prefix {
    public static void main(String[] args) {
        int n = 32;
        int block_size = 4;
        int[] array = generateArray(n);

        System.out.println("Random Array of size " + n);
        printArray(array);

        int[] result_seq = prefix(array);
        System.out.println("Result sequential: ");
        printArray(result_seq);
        
        int[] result_par = prefix_parallel(array, block_size);
        System.out.println("Result parallel: ");
        printArray(result_par);
    }

    private static void printArray(int[] array) {
        for (int el : array)
            System.out.print(el + ", ");
        System.out.println();
    }

    private static int[] generateArray(int n) {
        int[] array = new int[n];

        Random random = new Random();
        for (int i = 0; i < n; i++) {
            array[i] = random.nextInt(100);
        }

        return array;
    }

    private static class FirstStep extends Thread {
        int[] in, block_sum;
        int block_id, block_size;

        public FirstStep(int[] in, int[] block_sum, int block_id, int block_size) {
            this.in = in;
            this.block_sum = block_sum;
            this.block_id = block_id;
            this.block_size = block_size;
        }
        
        @Override
        public void run() {
            int block_offset = block_id * block_size;
            block_sum[block_id] = 0;

            for(int i = 0; i < block_size; i++)
                block_sum[block_id] += in[block_offset + i];
        }
    }

    private static class SecondStep extends Thread {
        int[] in, out, block_prefix;
        int block_id, block_size;

        public SecondStep(int[] in, int[] out, int[] block_prefix, int block_id, int block_size) {
            this.in = in;
            this.out = out;
            this.block_prefix = block_prefix;
            this.block_id = block_id;
            this.block_size = block_size;
        }

        @Override
        public void run() {
            int block_offset = block_size * block_id;
            out[block_offset] = block_prefix[block_id];

            for (int i = 1; i < block_size; i++) {
                out[block_offset + i] = out[block_offset + i - 1] + in[block_offset + i - 1];
            }
        }
    }

    public static int[] prefix_parallel(int[] in, int block_size) {
        int block_count = in.length / block_size;   // Annahme zur Vereinfachung in diesem
                                                    // Beispiel: in.length % block_size == 0
        int[] out = new int[in.length];
        int[] block_sum = new int[block_count];
        int[] block_prefix = new int[block_count];

        // 1. Schritt:
        Thread[] threads = new Thread[block_count];
        for(int block_id = 0; block_id < block_count; block_id++) {
            /*
            int block_offset = block_size * block_id;
            block_sum[block_id] = 0;
            for(int i = 0; i < block_size; i++)
                block_sum[block_id] += in[block_offset + i];
            */
            threads[block_id] = new FirstStep(in, block_sum, block_id, block_size);
            threads[block_id].start();
        }
        for (int block_id = 0; block_id < block_count; block_id++) {
            try {
                threads[block_id].join();
            } catch (Exception e) { 
                System.out.println( e.toString() );
            }
        }

        // 2. Schritt:
        block_prefix[0] = 0;
        for(int block_id = 1; block_id < block_count; block_id++)
            block_prefix[block_id] = block_prefix[block_id-1] + block_sum[block_id-1];

        // 3. Schritt:
        Thread[] threads2 = new Thread[block_count];
        for(int block_id = 0; block_id < block_count; block_id++) {
            /*
            int block_offset = block_size * block_id;
            out[block_offset] = block_prefix[block_id];
            for(int i = 1; i < block_size; i++)
                out[block_offset + i] = out[block_offset + i - 1] + in[block_offset + i - 1];
            */
            threads2[block_id] = new SecondStep(in, out, block_prefix, block_id, block_size);
            threads2[block_id].start();
        }
        for (int block_id = 0; block_id < block_count; block_id++) {
            try {
                threads2[block_id].join();
            } catch (Exception e) {
                System.out.println(e.toString());
            }
        }

        return out;
    }

    public static int[] prefix(int[] in) {
        int[] out = new int[in.length];
        out[0] = 0;
        for (int i = 1; i < in.length; i++) {
            out[i] = out[i-1] + in[i-1];
        }

        return out;
    }
}
