# Q2: Write a function has_sustained_fault(samples, threshold, n) that returns True if the signal 
# stays above threshold for n or more consecutive samples. Then write the tests for it.
import pytest


def has_sustained_fault(samples, threshold, n):
    if (n <= 0):
        raise ValueError("incorrect data")

    if (n > len(samples)):
        return False
    
    count = 0

    for i in range(len(samples)):
        if (isinstance(samples[i],(int, float)) and not isinstance(samples[i], bool)):
            if (samples[i] > threshold):
                count += 1
            else:
                count = 0 # Reset count

        else:
            raise ValueError("incorrect element")

        if (count >= n):
            return True
        
    return False

""" 
Testing cases:
1) elements in string are not numbers
2) n is 0 or greater than number of samples 
3) num of samples is 0


"""

@pytest.mark.parametrize("samples,threshold,n,expected", [
                         ([1,2,3,4],5,1,False), 
                         ([1,2,3,4],1,2,True), 
                         ([1,2,3,4],2,2,True), # towards the end
                         ([1,3,2,4],2,2,False), # (non-consecutive)
                         ([-1,-5.2,92,4,4.6],1,2,True), #  (involving floats)
                         ([],5,1,False), 
                         ([1,2,3,4],0,4,True), 
                         ([1,2,'g',4],0,2,True), # no problem, corrupted data comes after
                        ])
def test_samples(samples,threshold,n,expected):
    assert has_sustained_fault(samples,threshold,n) == expected

@pytest.mark.parametrize("n",[0,-5])
def test_invalid_n(n):
    samples = [1,2,3]
    threshold = 0
    with pytest.raises(ValueError,match="incorrect data"):
        has_sustained_fault(samples,threshold,n)

def test_incorrect_elem():
    with pytest.raises(ValueError,match="incorrect element"):
        has_sustained_fault([1,2,'g',3],0,3) 

