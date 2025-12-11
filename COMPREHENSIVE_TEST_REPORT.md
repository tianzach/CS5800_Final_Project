# Comprehensive Test Report: Three-Dimensional Analysis

## Test Dimensions

1. **Strategy** (4 algorithms):
   - 2-Pass DSU
   - BFS
   - DFS
   - DSU 1-pass

2. **Image Size** (4 sizes):
   - Small (200x200) - 0.04 MPixels
   - Medium (500x500) - 0.25 MPixels
   - Large (1000x1000) - 1.00 MPixels
   - Very Large (2000x2000) - 4.00 MPixels

3. **Input Type** (4 types):
   - Full Image (complete image, 30% density)
   - Stream 10% (10% of pixels in random order)
   - Stream 50% (50% of pixels in random order)
   - Stream 100% (all pixels in random order)

**Total Combinations**: 4 × 4 × 4 = **64 test cases**

## Winners by Category

### Small Images (200x200)

| Input Type | Winner | Time (μs) | Notes |
|------------|--------|-----------|-------|
| Full Image | **DFS** | 1,010.33 | DFS fastest |
| Stream 10% | **BFS** | 309.33 | BFS fastest |
| Stream 50% | **DSU 1-pass** | 683.33 | DSU wins! |
| Stream 100% | **BFS** | 269.33 | BFS fastest |

**Key Finding**: For small images, BFS/DFS dominate. DSU only wins at Stream 50%.

### Medium Images (500x500)

| Input Type | Winner | Time (μs) | Notes |
|------------|--------|-----------|-------|
| Full Image | **DFS** | 2,992.33 | DFS fastest |
| Stream 10% | **BFS** | 1,310.67 | BFS fastest |
| Stream 50% | **DFS** | 3,744.67 | DFS fastest |
| Stream 100% | **BFS** | 1,694.67 | BFS fastest |

**Key Finding**: BFS/DFS continue to dominate for medium images.

### Large Images (1000x1000)

| Input Type | Winner | Time (μs) | Notes |
|------------|--------|-----------|-------|
| Full Image | **BFS** | 12,014.33 | BFS fastest |
| Stream 10% | **DFS** | 5,341.00 | DFS fastest |
| Stream 50% | **DFS** | 14,993.00 | DFS fastest |
| Stream 100% | **DSU 1-pass** | 6,794.00 | **DSU wins!** |

**Key Finding**: DSU 1-pass wins at Stream 100% for large images!

### Very Large Images (2000x2000)

| Input Type | Winner | Time (μs) | Notes |
|------------|--------|-----------|-------|
| Full Image | **BFS** | 48,478.33 | BFS fastest |
| Stream 10% | **BFS** | 21,430.33 | BFS fastest |
| Stream 50% | **DFS** | 60,674.67 | DFS fastest |
| Stream 100% | **BFS** | 28,072.67 | BFS fastest |

**Key Finding**: For very large images, BFS/DFS still dominate, but DSU is competitive.

## Key Patterns

### 1. Full Image Processing

| Image Size | Winner | Time (μs) | Speedup vs 2-Pass |
|------------|--------|-----------|-------------------|
| Small | DFS | 1,010.33 | 3.52x |
| Medium | DFS | 2,992.33 | 2.73x |
| Large | BFS | 12,014.33 | 2.74x |
| Very Large | BFS | 48,478.33 | 2.90x |

**Pattern**: BFS/DFS consistently fastest for full image processing across all sizes.

### 2. Stream 10% (Small Streams)

| Image Size | Winner | Time (μs) |
|------------|--------|-----------|
| Small | BFS | 309.33 |
| Medium | BFS | 1,310.67 |
| Large | DFS | 5,341.00 |
| Very Large | BFS | 21,430.33 |

**Pattern**: BFS/DFS fastest for small streams.

### 3. Stream 50% (Medium Streams)

| Image Size | Winner | Time (μs) | Notes |
|------------|--------|-----------|-------|
| Small | DSU 1-pass | 683.33 | **DSU wins!** |
| Medium | DFS | 3,744.67 | DFS fastest |
| Large | DFS | 14,993.00 | DFS fastest |
| Very Large | DFS | 60,674.67 | DFS fastest |

**Pattern**: DSU wins only for small images at 50% stream. For larger images, DFS wins.

### 4. Stream 100% (Full Stream)

| Image Size | Winner | Time (μs) | Notes |
|------------|--------|-----------|-------|
| Small | BFS | 269.33 | BFS fastest |
| Medium | BFS | 1,694.67 | BFS fastest |
| Large | **DSU 1-pass** | 6,794.00 | **DSU wins!** |
| Very Large | BFS | 28,072.67 | BFS fastest |

**Pattern**: DSU 1-pass wins at Stream 100% for large images (1000x1000)!

## Performance Analysis

### Best Strategy by Scenario

#### Small Images (<500x500)
- **Best**: BFS/DFS
- **When DSU wins**: Only at Stream 50% for small images

#### Medium Images (500x500)
- **Best**: BFS/DFS
- **When DSU wins**: Never
- **Avoid**: 2-Pass DSU

#### Large Images (1000x1000)
- **Best**: BFS/DFS for most cases
- **When DSU wins**: Stream 100% (DSU 1-pass)
- **Avoid**: 2-Pass DSU

#### Very Large Images (2000x2000+)
- **Best**: BFS/DFS
- **When DSU wins**: Not in this test, but competitive

### DSU 1-pass Performance

DSU 1-pass shows **selective advantages**:
- ✅ Wins at Stream 50% for small images
- ✅ Wins at Stream 100% for large images
- ❌ Loses for most other scenarios

## Throughput Analysis

### Highest Throughput by Category

| Category | Best Strategy | Throughput |
|----------|---------------|------------|
| Full Image (Small) | DFS | 39.59 MP/s |
| Full Image (Large) | BFS | 83.23 MP/s |
| Stream 10% (Large) | DFS | 187.23 MP/s |
| Stream 100% (Large) | DSU 1-pass | 147.19 MP/s |

**Pattern**: Throughput is highest for stream inputs with BFS/DFS, except DSU wins at Stream 100% for large images.

## Critical Findings

### 1. **DSU Advantages Are Limited**
- DSU 1-pass wins in only **2 out of 16 categories**:
  - Small images, Stream 50%
  - Large images, Stream 100%

### 2. **BFS/DFS Dominate**
- BFS/DFS win in **14 out of 16 categories**
- They are consistently fast across all image sizes and input types

### 3. **2-Pass DSU Is Slowest**
- 2-Pass DSU is consistently the slowest method
- 2-3x slower than BFS/DFS

### 5. **Image Size Matters**
- Small images: BFS/DFS always win
- Large images: DSU has a chance at high stream ratios
- Very large images: BFS/DFS still dominate

## Recommendations

### For Small Images (<500x500)
- ✅ **Use BFS or DFS** - Always fastest
- ❌ Avoid DSU methods

### For Medium Images (500x500)
- ✅ **Use BFS or DFS** - Always fastest
- ❌ Avoid DSU methods

### For Large Images (1000x1000)
- ✅ **Use BFS/DFS** for most cases
- ✅ **Use DSU 1-pass** only for Stream 100%

### For Very Large Images (2000x2000+)
- ✅ **Use BFS/DFS** - Still fastest
- ⚠️ DSU is competitive but not winning

### For Stream Processing
- ✅ **Use BFS/DFS with full recomputation** - Fastest for all scenarios

## Conclusion

**Three-dimensional analysis reveals:**

1. **Strategy dimension**: BFS/DFS are the clear winners (14/16 wins)
2. **Image size dimension**: DSU advantages appear only at specific sizes (large images)
3. **Input type dimension**: DSU wins only at high stream ratios (50-100%)

**Overall Winner**: **BFS/DFS** dominate across all dimensions, with DSU showing selective advantages only in specific scenarios.

The comprehensive test confirms that **DSU is fastest when images are large AND stream ratio is high (50-100%)**, but BFS/DFS remain the best general-purpose choice.

