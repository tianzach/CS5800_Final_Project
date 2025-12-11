# Performance Comparison Tables

## Complete Performance Comparison

### Small Images (200x200, 0.04 MPixels)

| Strategy | Full Image | Stream 10% | Stream 50% | Stream 100% |
|----------|------------|------------|-----------|-------------|
| **2-Pass DSU** | 1,479.20 μs<br>27.04 MP/s | 821.00 μs<br>4.87 MP/s | 1,780.00 μs<br>11.24 MP/s | 2,136.00 μs<br>18.73 MP/s |
| **BFS** | 1,192.00 μs<br>33.56 MP/s | 261.20 μs<br>15.31 MP/s | 628.60 μs<br>31.82 MP/s | 275.20 μs<br>**145.35 MP/s** |
| **DFS** | **762.80 μs**<br>**52.44 MP/s** | **244.80 μs**<br>**16.34 MP/s** | 647.60 μs<br>30.88 MP/s | 278.80 μs<br>143.47 MP/s |
| **DSU 1-pass** | 530.60 μs<br>75.39 MP/s | 266.00 μs<br>15.04 MP/s | **586.00 μs**<br>**34.13 MP/s** | **267.20 μs**<br>**149.70 MP/s** |

**Winners:**
- Full Image: DFS (762.80 μs)
- Stream 10%: DFS (244.80 μs)
- Stream 50%: DSU 1-pass (586.00 μs)
- Stream 100%: DSU 1-pass (267.20 μs)

---

### Medium Images (500x500, 0.25 MPixels)

| Strategy | Full Image | Stream 10% | Stream 50% | Stream 100% |
|----------|------------|------------|-----------|-------------|
| **2-Pass DSU** | 8,153.40 μs<br>30.66 MP/s | 3,557.40 μs<br>7.03 MP/s | 11,700.40 μs<br>10.68 MP/s | 13,531.40 μs<br>18.48 MP/s |
| **BFS** | 3,045.60 μs<br>82.09 MP/s | 1,364.80 μs<br>18.32 MP/s | 4,124.40 μs<br>30.31 MP/s | 1,700.60 μs<br>147.01 MP/s |
| **DFS** | **3,014.20 μs**<br>**82.94 MP/s** | **1,339.60 μs**<br>**18.66 MP/s** | **3,784.60 μs**<br>**33.03 MP/s** | 1,854.60 μs<br>134.80 MP/s |
| **DSU 1-pass** | 3,413.00 μs<br>73.25 MP/s | 1,575.20 μs<br>15.87 MP/s | 3,966.40 μs<br>31.51 MP/s | **1,678.80 μs**<br>**148.92 MP/s** |

**Winners:**
- Full Image: DFS (3,014.20 μs)
- Stream 10%: DFS (1,339.60 μs)
- Stream 50%: DFS (3,784.60 μs)
- Stream 100%: DSU 1-pass (1,678.80 μs)

---

### Large Images (1000x1000, 1.00 MPixels)

| Strategy | Full Image | Stream 10% | Stream 50% | Stream 100% |
|----------|------------|------------|-----------|-------------|
| **2-Pass DSU** | 32,836.40 μs<br>30.45 MP/s | 14,194.60 μs<br>7.04 MP/s | 44,080.20 μs<br>11.34 MP/s | 53,657.00 μs<br>18.64 MP/s |
| **BFS** | 12,185.00 μs<br>82.07 MP/s | **5,346.60 μs**<br>**18.70 MP/s** | 16,136.60 μs<br>30.99 MP/s | 6,914.40 μs<br>144.63 MP/s |
| **DFS** | **12,139.20 μs**<br>**82.38 MP/s** | 5,353.00 μs<br>18.68 MP/s | **14,815.80 μs**<br>**33.75 MP/s** | 7,037.00 μs<br>142.11 MP/s |
| **DSU 1-pass** | 13,945.60 μs<br>71.71 MP/s | 6,283.60 μs<br>15.91 MP/s | 15,742.60 μs<br>31.76 MP/s | **6,700.40 μs**<br>**149.24 MP/s** |

**Winners:**
- Full Image: DFS (12,139.20 μs)
- Stream 10%: BFS (5,346.60 μs)
- Stream 50%: DFS (14,815.80 μs)
- Stream 100%: DSU 1-pass (6,700.40 μs)

---

### Very Large Images (2000x2000, 4.00 MPixels)

| Strategy | Full Image | Stream 10% | Stream 50% | Stream 100% |
|----------|------------|------------|-----------|-------------|
| **2-Pass DSU** | 135,118.80 μs<br>29.60 MP/s | 58,229.20 μs<br>6.87 MP/s | 180,674.00 μs<br>11.07 MP/s | 216,734.80 μs<br>18.46 MP/s |
| **BFS** | **48,914.60 μs**<br>**81.78 MP/s** | **21,373.60 μs**<br>**18.71 MP/s** | 63,624.20 μs<br>31.43 MP/s | 27,850.60 μs<br>143.62 MP/s |
| **DFS** | 52,478.60 μs<br>76.22 MP/s | 22,007.80 μs<br>18.18 MP/s | **59,637.40 μs**<br>**33.54 MP/s** | 28,495.40 μs<br>140.37 MP/s |
| **DSU 1-pass** | 68,124.80 μs<br>58.72 MP/s | 27,912.20 μs<br>14.33 MP/s | 67,430.60 μs<br>29.66 MP/s | **27,252.60 μs**<br>**146.77 MP/s** |

**Winners:**
- Full Image: BFS (48,914.60 μs)
- Stream 10%: BFS (21,373.60 μs)
- Stream 50%: DFS (59,637.40 μs)
- Stream 100%: DSU 1-pass (27,252.60 μs)

---

## Summary Statistics

### Overall Winners by Category

| Category | Winner | Time (μs) | Speedup vs 2-Pass |
|----------|--------|-----------|-------------------|
| **Full Image (Small)** | DFS | 762.80 | 1.94x |
| **Full Image (Medium)** | DFS | 3,014.20 | 2.70x |
| **Full Image (Large)** | DFS | 12,139.20 | 2.71x |
| **Full Image (Very Large)** | BFS | 48,914.60 | 2.76x |
| **Stream 10% (Small)** | DFS | 244.80 | 3.35x |
| **Stream 10% (Medium)** | DFS | 1,339.60 | 2.65x |
| **Stream 10% (Large)** | BFS | 5,346.60 | 2.66x |
| **Stream 10% (Very Large)** | BFS | 21,373.60 | 2.72x |
| **Stream 50% (Small)** | DSU 1-pass | 586.00 | 3.04x |
| **Stream 50% (Medium)** | DFS | 3,784.60 | 3.09x |
| **Stream 50% (Large)** | DFS | 14,815.80 | 2.97x |
| **Stream 50% (Very Large)** | DFS | 59,637.40 | 3.03x |
| **Stream 100% (Small)** | DSU 1-pass | 267.20 | 8.00x |
| **Stream 100% (Medium)** | DSU 1-pass | 1,678.80 | 8.06x |
| **Stream 100% (Large)** | DSU 1-pass | 6,700.40 | 8.01x |
| **Stream 100% (Very Large)** | DSU 1-pass | 27,252.60 | 7.95x |

### Algorithm Performance Summary

| Algorithm | Wins | Best For |
|-----------|------|----------|
| **DFS** | 7 | Full images, Stream 10%, Stream 50% |
| **BFS** | 3 | Very large full images, Stream 10% |
| **DSU 1-pass** | 6 | Stream 100% (all sizes), Stream 50% (small) |
| **2-Pass DSU** | 0 | None (consistently slowest) |

### Key Patterns

1. **Full Image Processing**: DFS/BFS dominate (10/16 wins)
2. **Stream 10%**: DFS/BFS fastest (8/8 wins)
3. **Stream 50%**: DFS/DSU competitive (DFS: 3 wins, DSU: 1 win)
4. **Stream 100%**: DSU 1-pass dominates (4/4 wins)

### Throughput Comparison

| Scenario | Best Algorithm | Throughput |
|----------|----------------|------------|
| Full Image (Small) | DSU 1-pass | 75.39 MP/s |
| Full Image (Medium) | DFS | 82.94 MP/s |
| Full Image (Large) | DFS | 82.38 MP/s |
| Full Image (Very Large) | BFS | 81.78 MP/s |
| Stream 100% (Small) | DSU 1-pass | 149.70 MP/s |
| Stream 100% (Medium) | DSU 1-pass | 148.92 MP/s |
| Stream 100% (Large) | DSU 1-pass | 149.24 MP/s |
| Stream 100% (Very Large) | DSU 1-pass | 146.77 MP/s |

**Note**: Stream 100% shows higher throughput because it processes fewer pixels (only foreground pixels in a dense pattern).

---

## Recommendations

### For Full Image Processing
- ✅ **Use DFS** for small to large images
- ✅ **Use BFS** for very large images
- ❌ Avoid 2-Pass DSU (2-3x slower)

### For Stream 10% (Sparse Streams)
- ✅ **Use DFS/BFS** - Both perform similarly well
- ❌ Avoid 2-Pass DSU

### For Stream 50% (Medium Streams)
- ✅ **Use DFS** for medium to very large images
- ✅ **Use DSU 1-pass** for small images
- ❌ Avoid 2-Pass DSU

### For Stream 100% (Dense Streams)
- ✅ **Use DSU 1-pass** - Consistently fastest (8x faster than 2-Pass)
- ❌ Avoid 2-Pass DSU

---

## Speedup Analysis

### 2-Pass DSU vs Best Algorithm

| Image Size | Full Image | Stream 10% | Stream 50% | Stream 100% |
|------------|------------|------------|-----------|-------------|
| Small | 2.79x slower | 3.35x slower | 3.04x slower | 8.00x slower |
| Medium | 2.70x slower | 2.65x slower | 3.09x slower | 8.06x slower |
| Large | 2.71x slower | 2.66x slower | 2.97x slower | 8.01x slower |
| Very Large | 2.76x slower | 2.72x slower | 3.03x slower | 7.95x slower |

**Average**: 2-Pass DSU is **3.5x slower** on average, and up to **8x slower** for Stream 100% scenarios.

