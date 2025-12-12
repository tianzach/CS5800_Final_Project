# make_mask.py
from __future__ import annotations
import numpy as np
from PIL import Image, ImageFilter, ImageOps


def otsu_threshold(gray: np.ndarray) -> int:
    """
    Compute Otsu threshold for 8-bit grayscale.
    """
    hist, _ = np.histogram(gray, bins=256, range=(0, 256))
    total = gray.size
    sum_total = np.dot(np.arange(256), hist)
    sum_b = 0.0
    w_b = 0.0
    var_max = 0.0
    threshold = 0

    for t in range(256):
        w_b += hist[t]
        if w_b == 0:
            continue

        w_f = total - w_b
        if w_f == 0:
            break

        sum_b += t * hist[t]
        m_b = sum_b / w_b
        m_f = (sum_total - sum_b) / w_f
        var_between = w_b * w_f * (m_b - m_f) ** 2

        if var_between > var_max:
            var_max = var_between
            threshold = t

    return threshold


def generate_mask(input_path: str,
                  output_path: str,
                  method: str = "otsu",
                  invert: bool = False,
                  smooth: int = 1,
                  morph_open: int = 1):
    """
    Generate a binary (0/1) mask from dermoscopy image.
    Saves PNG (0/255).
    """
    img = Image.open(input_path).convert("RGB")

    # --- Method 1: Gray threshold ---
    if method == "gray":
        gray = ImageOps.grayscale(img)
        arr = np.array(gray)
        thr = 150  # you can tune this
        mask = (arr < thr).astype(np.uint8)

    # --- Method 2: Otsu ---
    elif method == "otsu":
        gray = ImageOps.grayscale(img)
        if smooth > 0:
            gray = gray.filter(ImageFilter.GaussianBlur(radius=smooth))

        arr = np.array(gray, dtype=np.uint8)
        thr = otsu_threshold(arr)
        mask = (arr <= thr).astype(np.uint8)

    # --- Method 3: Darkness percentile ---
    elif method == "darkness":
        arr = np.array(img, dtype=np.uint8)
        lum = arr.mean(axis=2)
        thr = np.percentile(lum, 35)  # darker 35% → lesion
        mask = (lum <= thr).astype(np.uint8)

    else:
        raise ValueError("Unknown method")

    # Invert if needed
    if invert:
        mask = 1 - mask

    # --- Morphological Opening (optional) ---
    try:
        from scipy.ndimage import binary_opening
        if morph_open > 0:
            mask = binary_opening(mask.astype(bool),
                                  iterations=morph_open).astype(np.uint8)
    except Exception:
        pass

    # Save 0/255 PNG
    Image.fromarray(mask * 255).save(output_path)
