"""
def read_temperature(port):
    ...  # reads one float from a serial device

Write average_temperature(port, k) that takes k readings and returns the average, skipping 
any reading that is None (a failed read). If every reading fails, raise an error. Then write 
tests that don’t need real hardware.


"""
# This module is great for replacing hardware (external inputs)
from unittest.mock import patch
import pytest
from read_test import read_temperature 


def average_temp(port,k):
    if (port is None):
        raise TypeError("no COM detected")
    if (k <= 0):
        raise ValueError("invalid k")

    failed_reads = 0
    sum = 0
    for i in range(k):
        val = read_temperature(port)
        if (val == None):
            failed_reads += 1
        else:
            sum += val

    if (failed_reads == k):
        raise ValueError("every reading failed")
    
    return sum / (k - failed_reads)


# Want not to test error cases but valid cases
# Need to make sure the size of return_val is equal to k (the sensor should always give an output)
@pytest.mark.parametrize("k,return_val,expected",[
                        (5,[1,1,1,1,1],1), # normal test case -> avg is a integer
                        (5,[2,1,1,1,1],6/5), # avg is a float
                        (5,[None,1,2,3,6],3), # Include None, avg is integer 
                        (5,[None,1,2,3,4],2.5), # Include None, avg is float
                        (5,[1,None,3,None,None],2), # Include multiple Nones
                        (5,[1.5,None,3,None,5.2],97/30), # Include floats, integers, and None
                        (5,[-1.5,None,-3,None,5.2],7/30), # Negatives 
                        (4,[0.0,None,1,-2.3],-13/30), # test 0
                        (2,[3,None],3), # k = 1   
])
@patch("hardware.read_temperature") # File name then function name
def test_readings(mock_read,k,return_val,expected):
    mock_read.side_effect = return_val
    assert average_temp("COM1",k) == pytest.approx(expected) # abs is the tolerance
    assert mock_read.call_count == k


# Need to test if exceptions are raised
@patch("hardware.read_temperature") 
def test_failed_reading(mock_read):
    mock_read.side_effect = [None,None,None]
    with pytest.raises(ValueError,match="every reading failed"):
        average_temp("COM1",3)

# Test for invalid k
@pytest.mark.parametrize("k",[0,-1])
def test_failed_k(k):
    with pytest.raises(ValueError,match="invalid k"):
        average_temp("COM1",k)

# Test to see if COM connected
def test_COM():
    with pytest.raises(TypeError,match="no COM detected"):
        average_temp(None,1)

