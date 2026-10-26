import pytest
from fault_monitor import FaultMonitor, StatusCode

BAD, GOOD = (0.0, 10.0), (1.0, 1.0)     # tolerance 5, so BAD differs by 10

def feed(mon, pairs):
    for a, b in pairs:
        mon.update(a, b)

@pytest.fixture
def mon():
    return FaultMonitor(threshold=5, limit=3)

def test_starts_ok(mon):
    assert mon.state == StatusCode.OK

def test_limit_count_is_still_ok(mon):            # exactly `limit` bad samples
    feed(mon, [BAD] * 3)
    assert mon.state == StatusCode.OK

def test_over_limit_latches(mon):
    feed(mon, [BAD] * 4)
    assert mon.state == StatusCode.FAULT

def test_difference_equal_to_tolerance_is_ok(mon):
    feed(mon, [(0.0, 5.0)] * 10)
    assert mon.state == StatusCode.OK

def test_good_sample_breaks_streak(mon):
    feed(mon, [BAD] * 3 + [GOOD] + [BAD] * 3)
    assert mon.state == StatusCode.OK

def test_stays_latched_after_inputs_agree(mon):
    feed(mon, [BAD] * 4 + [GOOD] * 5)
    assert mon.state == StatusCode.FAULT

def test_reset_before_any_update(mon):
    assert mon.reset() is False

def test_reset_mid_streak_does_not_hide_fault(mon):
    feed(mon, [BAD] * 2)
    assert mon.reset() is False                   # no-op while OK
    feed(mon, [BAD] * 2)                          # streak of 4 > limit 3
    assert mon.state == StatusCode.FAULT

def test_reset_refused_while_still_disagreeing(mon):
    feed(mon, [BAD] * 4)
    assert mon.reset() is False
    assert mon.state == StatusCode.FAULT

def test_reset_succeeds_once_inputs_agree(mon):
    feed(mon, [BAD] * 4 + [GOOD])
    assert mon.reset() is True
    assert mon.state == StatusCode.OK

def test_count_starts_fresh_after_reset(mon):
    feed(mon, [BAD] * 4 + [GOOD])
    mon.reset()
    feed(mon, [BAD] * 3)
    assert mon.state == StatusCode.OK             # 3 is not > limit
    feed(mon, [BAD])
    assert mon.state == StatusCode.FAULT

@pytest.mark.parametrize("bad", ["g", None, True, float("nan"), float("inf")])
def test_invalid_sample_rejected(mon, bad):
    with pytest.raises(ValueError, match="invalid channel inputs"):
        mon.update(bad, 1.0)

@pytest.mark.parametrize("tol,limit,msg", [(-5, 2, "invalid threshold"),
                                           (5, 0, "invalid limit")])
def test_invalid_constructor(tol, limit, msg):
    with pytest.raises(ValueError, match=msg):
        FaultMonitor(tol, limit)