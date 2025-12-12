#!/usr/bin/env python3
"""
Test suite for Connected Component Labeling algorithms
"""
import numpy as np
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from dsu_2pass import label_cc_2pass
from algorithms import label_cc_bfs, label_cc_dfs, label_cc_dsu


def test_simple_4_connected():
    """Test simple 4-connected case"""
    img = np.array([
        [1, 1, 0, 0],
        [1, 0, 0, 0],
        [0, 0, 1, 1],
        [0, 0, 1, 1]
    ], dtype=np.uint8)
    
    result_2pass = label_cc_2pass(img, eight_connectivity=False)
    result_bfs = label_cc_bfs(img, eight_connectivity=False)
    result_dfs = label_cc_dfs(img, eight_connectivity=False)
    result_dsu = label_cc_dsu(img, eight_connectivity=False)
    
    # Should have 2 components
    assert len(np.unique(result_2pass[result_2pass > 0])) == 2, "2-pass failed"
    assert len(np.unique(result_bfs[result_bfs > 0])) == 2, "BFS failed"
    assert len(np.unique(result_dfs[result_dfs > 0])) == 2, "DFS failed"
    assert len(np.unique(result_dsu[result_dsu > 0])) == 2, "DSU failed"
    
    # All should have same number of components
    assert (np.unique(result_2pass[result_2pass > 0]).size == 
            np.unique(result_bfs[result_bfs > 0]).size), "Mismatch"
    
    print("✓ Simple 4-connected test passed")


def test_simple_8_connected():
    """Test simple 8-connected case"""
    img = np.array([
        [1, 0, 1],
        [0, 1, 0],
        [1, 0, 1]
    ], dtype=np.uint8)
    
    # With 8-connectivity, should be 1 component
    result_2pass = label_cc_2pass(img, eight_connectivity=True)
    result_bfs = label_cc_bfs(img, eight_connectivity=True)
    
    assert len(np.unique(result_2pass[result_2pass > 0])) == 1, "2-pass 8-conn failed"
    assert len(np.unique(result_bfs[result_bfs > 0])) == 1, "BFS 8-conn failed"
    
    # With 4-connectivity, should be 5 components
    result_2pass_4 = label_cc_2pass(img, eight_connectivity=False)
    assert len(np.unique(result_2pass_4[result_2pass_4 > 0])) == 5, "2-pass 4-conn failed"
    
    print("✓ Simple 8-connected test passed")


def test_empty_image():
    """Test empty (all background) image"""
    img = np.zeros((10, 10), dtype=np.uint8)
    
    result_2pass = label_cc_2pass(img)
    result_bfs = label_cc_bfs(img)
    
    assert np.all(result_2pass == 0), "2-pass empty failed"
    assert np.all(result_bfs == 0), "BFS empty failed"
    
    print("✓ Empty image test passed")


def test_full_foreground():
    """Test full foreground image"""
    img = np.ones((5, 5), dtype=np.uint8)
    
    result_2pass = label_cc_2pass(img)
    result_bfs = label_cc_bfs(img)
    
    # Should have exactly 1 component
    assert len(np.unique(result_2pass[result_2pass > 0])) == 1, "2-pass full failed"
    assert len(np.unique(result_bfs[result_bfs > 0])) == 1, "BFS full failed"
    
    print("✓ Full foreground test passed")


def test_consistency():
    """Test that all algorithms produce consistent results"""
    np.random.seed(42)
    img = (np.random.rand(50, 50) < 0.3).astype(np.uint8)
    
    result_2pass = label_cc_2pass(img)
    result_bfs = label_cc_bfs(img)
    result_dfs = label_cc_dfs(img)
    result_dsu = label_cc_dsu(img)
    
    n_2pass = len(np.unique(result_2pass[result_2pass > 0]))
    n_bfs = len(np.unique(result_bfs[result_bfs > 0]))
    n_dfs = len(np.unique(result_dfs[result_dfs > 0]))
    n_dsu = len(np.unique(result_dsu[result_dsu > 0]))
    
    assert n_2pass == n_bfs == n_dfs == n_dsu, \
        f"Component count mismatch: 2pass={n_2pass}, BFS={n_bfs}, DFS={n_dfs}, DSU={n_dsu}"
    
    print("✓ Consistency test passed")


def run_all_tests():
    """Run all tests"""
    print("Running tests...\n")
    try:
        test_simple_4_connected()
        test_simple_8_connected()
        test_empty_image()
        test_full_foreground()
        test_consistency()
        print("\n✅ All tests passed!")
        return 0
    except AssertionError as e:
        print(f"\n❌ Test failed: {e}")
        return 1
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(run_all_tests())

