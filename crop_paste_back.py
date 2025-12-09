import numpy as np

def crop_to_bbox(mask: np.ndarray):
    ys, xs = np.where(mask == 1)
    if ys.size == 0:
        return mask.copy(), (0,0,mask.shape[0],mask.shape[1])
    y1, y2 = ys.min(), ys.max()+1
    x1, x2 = xs.min(), xs.max()+1
    return mask[y1:y2, x1:x2].copy(), (y1, x1, y2, x2)

def paste_back(labels_small: np.ndarray, shape, bbox):
    H, W = shape
    y1, x1, y2, x2 = bbox
    out = np.zeros((H,W), dtype=np.int32)
    out[y1:y2, x1:x2] = labels_small
    return out
