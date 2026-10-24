// Given a non-empty array of integers where every element appears exactly three times,
// except for one element which appears exactly once, find that single element.

int single_number(int *nums, int n) {
    int ans = 0;

    for (int j=0; j<32; j++) {  // 32n times 
        int bit_sum = 0;
        for (int i=0; i<n; i++) {
            bit_sum += (nums[i] & (1u << j)) >> j;        
        }
        if (bit_sum % 3 == 1) { // that means the corresponding bit in our target is not set 
            ans |= (1 << j);
        }
    }

    return ans;
}



