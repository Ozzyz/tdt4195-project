import numpy as np
import cv2
from matplotlib import pyplot as plt
from region_growing import *

def find_contours(img):
    """Return contours of a grayscale image
    """
    img_copy = img.copy()
    im2, contours, h = cv2.findContours(img_copy, 1, 2)
    return contours

def draw_contours(img, contours):
    """Return copy of input image with contours drawn
    """
    img_copy = img.copy()
    cv2.drawContours(img_copy, contours, -1, (0, 255, 0), 3)
    return img_copy

if __name__ == "__main__":
    img = cv2.imread("images/easy01.png")
    #gray = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
    seeds = [(355,150), (355, 254), (355, 43), (434, 22), (760, 39), (746, 70), (742, 26), (759, 438), (145, 148), (549, 153), (660, 252), (251, 346), (357, 349), (652, 355), (52, 451), (455, 447)]
    grown = region_grow(img, seeds, 40)
    c = find_contours(grown)
    c_img = draw_contours(img, c)
    plt.figure()
    plt.imshow(c_img, cmap=plt.cm.gray)
    plt.show()

