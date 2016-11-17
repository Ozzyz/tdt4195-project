import cv2
import numpy as np
from matplotlib import pyplot as plt
from thresh import blur_threshold_close

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
    
img = cv2.imread('images/easy01.png')
new_img = blur_threshold_close(img)

template_fp = 'images/pacman.png'
template_multiple_regions(img, new_img,  template_fp)
