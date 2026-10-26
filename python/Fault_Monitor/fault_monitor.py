"""
Q4 (the last big type, and the one most tied to your resume): Write a FaultMonitor class with a 
method update(ch_a, ch_b) called once per sample. It tracks two redundant sensor channels. If the 
channels differ by more than a tolerance for more than limit consecutive samples, it enters a latched
FAULT state, and it stays latched until reset() is called. Then write tests.


"""
from enum import Enum, unique
import math

@unique
class StatusCode(Enum):
    OK = 0
    FAULT = 1

# variables: state, thershold, limit 
class FaultMonitor:
    def __init__(self,threshold,limit): # threshold is absolute not relative
        if (limit < 1):
            raise ValueError("invalid limit")
        if (threshold < 0):
            raise ValueError("invalid threshold")
        
        
        self._state = StatusCode.OK
        self.threshold = threshold
        self.limit = limit
        self._current_count = 0
        self._current_read_ok = True

    @property
    def state(self):
        return self._state
    
    @property
    def current_count(self):
        return self._current_count

    @property
    def current_read_ok(self):
        return self._current_read_ok
    
    def update(self,ch_a,ch_b): # Assume Update is called every sample and that i will always receive two samples
        if (((ch_a in (float("inf"),float("nan"))) or not isinstance(ch_a,(int,float)) or isinstance(ch_a,bool)) or
            ((ch_b in (float("inf"),float("nan"))) or not isinstance(ch_b,(int,float)) or isinstance(ch_b,bool))):
            raise ValueError("invalid channel inputs") 

        if ((not math.isfinite(ch_a) or not math.isfinite(ch_b)) or (abs(ch_a - ch_b) > self.threshold)):
            self._current_count += 1
            self._current_read_ok = False
        else:
            self._current_count = 0
            self._current_read_ok = True

        if (self.current_count > self.limit):
            self._state = StatusCode.FAULT

        

# Not really a state machine, point of reset is to see if the user managed the fault
# if fault and current reading is ok -> true, the system can be safely reset
# if fault and current reading not ok -> false, system is not safe to reset
# if no fault -> false (i could be in a streak and if i reset now, i will lose it)
    def reset(self):
        if (self._state == StatusCode.OK):
            return False #no need for reset
        
        if (self._current_read_ok and (self._state == StatusCode.FAULT)):
            self._current_count = 0
            self._state = StatusCode.OK
            return True
        elif (not self._current_read_ok and (self._state == StatusCode.FAULT)):
            return False # just assume in order to reset the system you have to wait for a good sample


