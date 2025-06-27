/*
 * 1 2 | 3 4 | 5 6 | 7 8 Array
 * 3 7 11 15 Blocksummen: Parallel
 * 3 10 21 36 Prefixsummen für die Blocksummen: Sequentiell
 * 0+1 0+1+2 | 3+3 3+3+4 | 10+5 10+5+6 | 21+7 21+7+8 Prefixsummen: Parallel
 */

import java.util.concurrent.ThreadLocalRandom;
import java.util.Arrays;

public class PrefixSumCalculator {
    private static int[] prefixSumSequential(int[] in) {
        int[] out = new int[in.length];

        out[0] = 0;
        for (int i = 1; i < in.length; ++i)
            out[i] = out[i - 1] + in[i - 1];

        return out;
    }

    private class Shared {
        int[] in;
        int[] out;
        int[] blockSum;
        int[] blockPrefix;
        int blockCount;
        int blockSize;
    }

    private Shared shared = new Shared();

    private class SumCalculator extends Thread {
        int block_id;
        int start_index;

        public SumCalculator(int block_id, int start_index) {
            this.block_id = block_id;
            this.start_index = start_index;
        }

        @Override
        public void run() {
            shared.blockSum[block_id] = 0;
            for (int i = this.start_index; i < this.start_index + shared.blockSize; ++i)
                shared.blockSum[block_id] += shared.in[i];
        }
    }

    private class BlockPrefixSumCalculator extends Thread {
        int block_id;
        int start_index;

        public BlockPrefixSumCalculator(int block_id, int start_index) {
            this.block_id = block_id;
            this.start_index = start_index;
        }

        @Override
        public void run() {
            shared.out[start_index] = shared.blockPrefix[block_id];
            for (int i = start_index + 1; i < start_index + shared.blockSize; ++i)
                shared.out[i] = shared.out[i - 1] + shared.in[i - 1];
        }
    }

    private int[] prefixSumParallel(int[] in, int blockSize) {
        if (in.length % blockSize != 0)
            throw new RuntimeException("Array must be evenly dividable by the block size");

        shared.in = in;
        shared.blockSize = blockSize;
        shared.blockCount = in.length / blockSize;
        shared.out = new int[in.length];
        shared.blockSum = new int[shared.blockCount];
        shared.blockPrefix = new int[shared.blockCount];

        Thread[] threads = new Thread[shared.blockCount];
        for (int i = 0; i < shared.blockCount; ++i) {
            threads[i] = new SumCalculator(i, i * blockSize);
            threads[i].start();
        }

        for (int i = 0; i < shared.blockCount; ++i) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
            }
        }

        shared.blockPrefix[0] = 0;
        for (int i = 1; i < shared.blockCount; ++i)
            shared.blockPrefix[i] = shared.blockPrefix[i - 1] + shared.blockSum[i - 1];

        for (int i = 0; i < shared.blockCount; ++i) {
            threads[i] = new BlockPrefixSumCalculator(i, i * blockSize);
            threads[i].start();
        }

        for (int i = 0; i < shared.blockCount; ++i) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
            }
        }

        return shared.out;
    }

    public static void main(String[] args) {
        int[] array = new int[100];
        int[] blockSizes = { 2, 5, 10 };

        for (int i = 0; i < array.length; ++i)
            array[i] = ThreadLocalRandom.current().nextInt(1000);

        for (int i = 0; i < blockSizes.length; ++i) {
            int[] correctPrefixSum = prefixSumSequential(array);

            PrefixSumCalculator calculator = new PrefixSumCalculator();
            int[] parallelPrefixSum = calculator.prefixSumParallel(array, blockSizes[i]);

            if (!Arrays.equals(correctPrefixSum, parallelPrefixSum))
                throw new RuntimeException("Arrays are not equal!");
        }

        System.out.println("OK");
    }
}