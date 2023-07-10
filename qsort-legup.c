/**
 * Implemented non-recursive quick sort, adapted from this on-line implementation:
 *     http://alienryderflex.com/quicksort/
 * This benchmark calls the qsort 20 times, each on an array of 50 items.
 *
 * The sort data was generated by drawing numbers from a Gaussian distribution
 * with mean=10000, stdev=5000.
 */

#include <stdio.h>

int sortData[20][50] = {
  5924, 5021, 23098, 16305, 17160, 10976, 9128, 10580, 7711, 12660,
  382, 2623, 11065, 12248, 6311, 7117, 12376, 7878, 3757, 10610, 18980,
  6406, 6366, 9829, 14604, 9345, 18064, 6037, 14074, 6547, 10207, 14959,
  11052, 6857, 13624, 6737, 7443, 16391, 8476, 7430, 12105, 2736, 6773,
  7791, -220, 9613, 13787, 11612, 7104, 2786, 11148, 13040, 11974,
  18414, 9703, 10233, 12913, 10669, 6459, 11442, 12854, 13376, 7947,
  10652, 20890, 10174, 10302, 3622, 10196, 8154, 11198, 12389, 17108,
  6140, 15429, 8213, 3897, 10695, 10360, -637, 13474, 9626, 8353, 6225,
  8104, 10566, 2764, 4685, 16487, 4797, 17884, 7190, 10257, -1266,
  11835, 4607, 18564, 3578, 13754, 2708, 15007, 6082, 9342, 697, 9931,
  10923, 5457, 4072, 11407, 12924, 13969, 4922, 10354, 15544, 4269,
  23411, 19168, 8456, 7330, 9717, 10675, 6997, 17970, 617, 11911, 7853,
  15366, 2797, 9501, 7257, 9288, 12108, 6933, 3269, 6716, 6288, 17093,
  11034, 5880, 11259, 10113, 4117, 6351, 13298, 13515, 12714, 3716,
  12284, 7093, 2627, 15307, 7169, 2550, 11562, 13964, 8308, -734, 10964,
  16269, -1209, 440, 14295, 14445, 10205, 12401, 16333, 13204, 6681,
  15435, 7539, 10033, 3698, 17026, 7184, 11132, 4110, 18973, 11232,
  10243, 12272, 5809, 8963, 9207, 14236, 3675, 11382, 14635, 13822,
  10773, 12318, 3688, 10173, 15247, 9214, 8818, 5584, 14938, 4445,
  16364, 15284, 17641, 17088, 6790, 1693, 10677, -998, 6689, 1398, 4783,
  12829, 11503, 8062, 6469, 15270, 15798, 1623, 14085, 19451, -1106,
  8609, 8082, 15574, 16911, 3244, 11849, 6429, 13344, 6681, 4859, 3391,
  14596, 6900, 12543, 16699, 2999, 18484, 10456, 19317, 5669, 13376,
  16548, 5689, 11125, 9091, 6244, 16100, 16400, 9788, 14626, 9435,
  13037, 6162, 7156, 6100, 6609, 4857, 12520, 3070, 17418, 10047, 13476,
  12441, 5394, -1332, 11036, 7877, 11844, 3461, 4452, 3790, 4702, 14337,
  7187, 9772, 11060, 6943, 16266, 7858, 12902, 11477, 6297, 12186, 5459,
  4732, 13520, 7555, 14646, 17423, 16716, 12589, 12196, 9638, 12644,
  7646, -3182, 9392, 5061, 2380, 2332, 14170, 11057, 9617, 13656, 17269,
  12466, 10479, 15020, 7199, 12799, 10520, 2006, 16232, 7356, 9545,
  10179, 15857, 9510, 14189, 3654, 14495, 15943, 8965, 9662, 6839,
  18589, 16998, -1820, 13122, 11845, 9596, 6455, 3818, 13472, -6076,
  8172, 4118, 17991, 11461, 12377, 7836, 20496, 11788, 14747, 10405,
  2478, 7461, 5313, 8899, 13494, 19134, -954, 14471, 7982, 6666, 5523,
  4346, 6120, 7487, 11482, 4568, 9582, 10191, 19177, 20985, 11977,
  15595, 4671, 3590, 18311, 3829, 3745, 6583, 16475, 11662, 3389, 11257,
  12508, -3245, 3153, 2361, 7664, 21447, 21457, 15699, 8552, 5432,
  11895, 3063, 3550, 9384, 3972, -2382, 12911, 12844, 10667, 8310, 8860,
  7175, 10681, 15139, 14647, 15284, 9212, 14778, 13039, 7952, 13989,
  7426, 8974, 8167, 8851, -1093, 20460, 8586, 15614, 9974, 19948, 10316,
  16187, 16453, 2910, 7209, 11611, 15983, 1869, 6799, 10921, 15373,
  5704, 11251, 9594, 12083, 9044, 15710, 7965, 3886, 11596, 4775, 2806,
  15052, 5279, 10335, 10849, 16370, 6186, 13744, 1283, 11996, 4531,
  7194, 9533, 13213, 5464, 8313, 3408, 8951, 16397, 12120, 11907, 9517,
  12217, 8082, 7825, 10547, -1468, 7856, 11246, 12788, 7935, 10392,
  3728, 7829, 2717, 16940, 10984, 5474, 9643, 10040, 8989, 8415, 7287,
  9584, 4098, 11665, 2654, 10992, 19064, 12604, 11294, 7908, 9011, 4468,
  15939, 7151, 3541, 18828, 14533, 11419, 6840, 6026, 15903, 7381, 6384,
  8825, 14058, 8533, 6657, 16735, 8664, 11332, 8096, 8923, 18998, 21289,
  12599, 12275, 15905, 8998, 18068, 15474, 18679, 8925, 2975, 14057,
  10720, -2995, 16578, 14989, 17290, 9905, 20438, 1358, 10868, 7948,
  13782, 7266, 12011, 8596, 10157, 17470, 14552, 14904, 11849, 16179,
  10569, 4091, 6330, 15568, 16999, 11190, 11815, 7112, 14124, 7781,
  18528, 14085, 10376, 5049, 15912, 11971, 15573, 10712, 23225, 14644,
  11487, 7263, -1599, 14999, 7981, 8458, 14765, 5287, 14647, -150,
  12430, 743, 1986, 9885, 15238, 5204, 8537, 7353, 10355, 3285, 9025,
  18195, 12454, 19247, 13503, 10686, 3960, 12859, 12294, 5323, 3339,
  9174, -4869, 17424, 18243, 3162, 8541, 5610, 21959, 2517, 13682,
  16482, 7140, 14570, 15401, 13017, 6099, 9087, 19998, 10945, 12846,
  5682, 814, 3913, 11644, 8070, 8793, 15768, 10299, 10423, 16895, 4857,
  556, 4318, 15302, 14528, -648, 8589, 5138, 10426, 12908, 10422, 7800,
  14190, 9764, 19131, 8266, 15544, 8811, 10576, 16895, 5223, 2073,
  12102, 6804, 14754, 15708, 11416, 14178, 17885, 7051, 8831, 4557,
  14043, 7499, 5308, 5827, 11645, 9112, 16521, 9865, 14337, 11263, 8197,
  6005, 18561, 17659, 13698, 13281, 6703, 18010, 5266, 11796, 19074,
  15395, 10111, 6733, 6303, 19733, 4759, 12510, 11824, 5553, 19392,
  15491, 15352, 15992, 11801, 11345, 14089, 15701, 13057, 6505, 3952,
  8519, 8297, 4245, 6752, 6302, 17859, 9041, 6493, 4773, 6773, 8611,
  13898, 17519, -2805, 9609, 18153, 16846, 9401, 3362, 16261, 11918,
  7557, 12576, 13117, 9052, 5744, 14877, 7263, -574, 9108, 7252, 1339,
  9780, 7887, 15004, 11122, 12440, 11314, 8487, 16712, 14462, 6575,
  5242, 4985, 3262, 6450, 10457, 9545, 3926, 5196, 5398, 9122, 12124,
  7502, 6579, 9520, 11206, 12245, 16118, 15025, 13514, 12022, 5991,
  6003, 2816, 10057, 7215, 16221, 3627, 18569, 14575, 8139, 16290,
  12449, 14779, 10756, 8395, 12138, 18606, 6287, 9144, 9142, 7839,
  11030, 11131, 13523, 13077, 3754, 6787, 10575, 8610, 9100, 9933,
  13737, 8374, 9043, 12715, 402, 1909, 13098, 2566, 16800, 10991, 10993,
  2740, 9628, 17012, 19433, 13485, 13863, 7522, 4338, 7250, 17297, 4233,
  8883, 12119, 13032, 8052, 10092, 12135, 14963, 4278, 12978, 12476,
  9187, 13908, 3108, 4365, 13701, 7008, 11304, 10460, 8598, 11398, 2083,
  13092, 17065, 8445, 3407, 8282, 11708, 11319, 204, 12704, 6707, -782,
  9785, 13591, 8425, 11199, 17536, 9477, 12735, 14294, 17323, 8822,
  11065, 11487, 5685, 8656, -6302, 8039, 12400, 8217, 11934, 10690,
  13321, 8290, 20121, 10312, 7526, 19224, 15933, 6345, -532, 19164,
  9853, 15289, 12561, 14038, 8178, 12916, 12326, 8222, 14984, 17651,
  9478, -729, 10908, 12412, 12746, 11529, 11124, 6787, 12763, 12200,
  10753, 13291, 3698, 10774, 10566, 12204, 10699, 3671, 13584, 6827,
  15857, 15394, 9900, 582, 6026, 1312, 11571, 6979, 10980, 17291, 25654,
  10207, 15288, 1675, 13814, 4161, 7944, 8521, 9499, 2660, 2480, 15942,
  10327, 4989, 655, 3058, 10188, 9885, 13085, 10676, 14665, 7001, 5187,
  5032, 13904, 12711, 12033, 10485, 11273, 11984, 11923, 7747, 11185,
  13008, 8259, 9034, 10994, 2295, 10960, 11839, 1767, 13969, 8522,
  11860, 22623, 8535, 17982, 6981, 13625, 18628, -479, 19727, 13571,
  16668, 13417, 13602, 13220, 4882, 9370, 16600, 9657, 13989, 1227,
  7041, 8790, 10930, 3546, 3880, 11553, 7534, 10407, 5218, 8105, 9694,
  6842, 12005, 12823, 14191, 7823, 8506, 9637, 7498, 18137, 9794, 9648};

void quickSort(int *arr, int elements) {

  #define  MAX_LEVELS  300

  int  piv, beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap ;

  beg[0]=0; end[0]=elements;
  while (i>=0) {
    L=beg[i]; R=end[i]-1;
    if (L<R) {
      piv=arr[L];
      while (L<R) {
        while (arr[R]>=piv && L<R) R--; if (L<R) arr[L++]=arr[R];
        while (arr[L]<=piv && L<R) L++; if (L<R) arr[R--]=arr[L]; }
      arr[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
      if (end[i]-beg[i]>end[i-1]-beg[i-1]) {
        swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
        swap=end[i]; end[i]=end[i-1]; end[i-1]=swap; }}
    else {
      i--; 
    }
  }
}

int main(int argc, char ** argv)
{
  int i,j;
  /* call quickSort 20 times, each with a 50 element array */
  for (i = 0; i < 20; i++)
    quickSort(sortData[i], 50);
  
  /* now check each array of 50 elements is indeed in sorted order */
  int correct = 0;
  for (i = 0; i < 20; i++) {
    for (j = 1; j < 50; j++) {
      if (sortData[i][j] >= sortData[i][j-1]) {
        correct++;
      }
    }
  }
  
  printf("Result: %d\n", correct);

  if (correct == 980) {
    printf("RESULT: PASS\n");
  } else {
    printf("RESULT: FAIL\n");
  }
  /* return correct; */
  return 0;
}
