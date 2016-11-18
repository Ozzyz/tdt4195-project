import cv2
import numpy as np
from matplotlib import pyplot as plt
from thresh import blur_threshold_close
from queue import Queue as Q
def template_single_region():
    img = cv2.imread('images/easy01.png',0)
    img2 = img.copy()
    template = cv2.imread('images/hexagon.png',0)
    w, h = template.shape[::-1]

    # All the 6 methods for comparison in a list
    methods = ['cv2.TM_CCOEFF', 'cv2.TM_CCOEFF_NORMED', 'cv2.TM_CCORR',
                'cv2.TM_CCORR_NORMED', 'cv2.TM_SQDIFF', 'cv2.TM_SQDIFF_NORMED']

    for meth in methods:
        img = img2.copy()
        method = eval(meth)

        # Apply template Matching
        res = cv2.matchTemplate(img,template,method)
        min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)

        # If the method is TM_SQDIFF or TM_SQDIFF_NORMED, take minimum
        if method in [cv2.TM_SQDIFF, cv2.TM_SQDIFF_NORMED]:
            top_left = min_loc
        else:
            top_left = max_loc
        bottom_right = (top_left[0] + w, top_left[1] + h)

        cv2.rectangle(img,top_left, bottom_right, 255, 2)

        plt.subplot(121),plt.imshow(res,cmap = 'gray')
        plt.title('Matching Result'), plt.xticks([]), plt.yticks([])
        plt.subplot(122),plt.imshow(img,cmap = 'gray')
        plt.title('Detected Point'), plt.xticks([]), plt.yticks([])
        plt.suptitle(meth)
        plt.show()

import cv2
import numpy as np
from matplotlib import pyplot as plt

def template_multiple_regions(img_rgb, img_thresh, template_fp): 
    
   # img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    template = cv2.imread(template_fp,0)
    w, h = template.shape[::-1]

    res = cv2.matchTemplate(img_thresh,template,cv2.TM_CCOEFF_NORMED)
    print(res)
    # TODO: Set this, so that every shape will be marked by templating
    threshold = 0.35
    loc = np.where( res >= threshold)
    print(str(loc[0]))
    for  pt in zip(*loc[::-1]):
        cv2.rectangle(img, pt, (pt[0] + w, pt[1] + h), (0,0,255), 2)
    plot_image(img_thresh)
    #cv2.imwrite('res.png',img)


def plot_image(image, cmap=plt.cm.gray):
    plt.figure()
    plt.imshow(image, cmap=cmap)
    plt.show()

def region_grow(img, seeds, thresh):
    
    new_img = np.zeros_like(img)
    new_img = new_img[..., 0]
    for seed in seeds:
        visited = set()
        queue = Q()
        queue.put(seed)
        print("===============")
        print("SEED {}".format(seed))
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
                            print("{} > {}".format(diff, thresh))
                            is_in_thresh = False
                        else:
                            print("{} < {}".format(diff, thresh))
                        
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
                            
seeds = [(355,150), (355, 254), (355, 43), (434, 22), (760, 39), (746, 70), (742, 26), (759, 438), (145, 148), (549, 153), (660, 252), (251, 346), (357, 349), (652, 355), (52, 451), (455, 447)]        
img = cv2.imread('images/easy01.png')
#blur = cv2.GaussianBlur(img, (3,3), 0)
img = cv2.fastNlMeansDenoisingColored(img, None, 10, 10, 7, 21)
new_img = region_grow(img, seeds, 40)
print(new_img.shape)
#print(new_img)
plt.figure()

plt.imshow(new_img, cmap=plt.cm.gray)
plt.show()
