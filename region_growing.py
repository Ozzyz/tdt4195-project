import cv2
import numpy as np
from matplotlib import pyplot as plt
from thresh import blur_threshold_close
from queue import Queue as Q
import cv2
import numpy as np
from matplotlib import pyplot as plt
from contours import *

def plot_image(image, cmap=plt.cm.gray):
    plt.figure()
    plt.imshow(image, cmap=cmap)
    plt.show()

def region_grow(img, seeds, thresh):
    
    new_img = np.zeros_like(img)
    new_img = new_img[..., 0]
    visited = set()
    for seed in seeds:
        queue = Q()
        queue.put(seed)
        x0, y0 = seed
        # RGB channel of original img
        img_seed_val = img[y0, x0]
        new_img[y0][x0] = 1
        # Visit each neighbour of seed
        while not queue.empty():
            curr_pixel = queue.get()
            # Visit each pixel of neighbour
            for neighbour in get_neighbours(curr_pixel, img):
                i, j = neighbour
                # If we haven't checked this neighbour before
                if neighbour not in visited:
                    visited.add(neighbour)
                    
                    # Check each color channel
                    is_in_thresh = True
                    for c in range(3):
                        seed_val = img_seed_val[c]
                        
                        neighbour_val = img[j][i][c]
                        if seed_val >= neighbour_val:
                            diff = seed_val - neighbour_val
                        else:
                            diff = neighbour_val - seed_val
                        if diff > thresh:
                           is_in_thresh = False                 
                    if is_in_thresh:
                        # Visit this neighbour later
                        queue.put(neighbour)
                        new_img[j][i] = 1
                        
    return new_img

def get_neighbours(pixel, img):
    neighbours = set()
    x,y = pixel

    for i in range(x-1, x+2):
        for j in range(y-1, y+2):
            if (i == x and j == y) or i not in range(img.shape[1]) or j not in range(img.shape[0]):
                continue
            else:
                neighbours.add((i, j))
    return neighbours
                            
if __name__ == "__main__":
    
    seeds = [(355,150), (355, 254), (355, 43), (434, 22), (760, 39), (746, 70), (742, 26), (759, 438), (145, 148), (549, 153), (660, 252), (251, 346), (357, 349), (652, 355), (52, 451), (455, 447)]        
    img = cv2.imread('images/easy01.png')
    img = cv2.fastNlMeansDenoisingColored(img, None, 10, 10, 7, 21)

    new_img = region_grow(img, seeds, 40)
    plot_image(new_img)
    _, new_img = cv2.threshold(new_img,127,255,0)
    cont = find_contours(new_img)
    cont_image = draw_contours(new_img, cont)
    plot_image(cont_image)
